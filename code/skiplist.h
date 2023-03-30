/*
 * @Description: c++实现简易跳表
 * @Autor: KylinLzw
 * @LastEditTime: 2023-03-30 09:17:07
 */

#ifndef SKIPLIST_H
#define SKIPLIST_H

#include <iostream>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <mutex>
#include <fstream>
#include <time.h>

using namespace std;

#define STORE_FILE "../store/dumpFIle"

std::mutex mtx;
std::string delimiter = ":";

template<typename T>
struct Less {
    bool operator () (const T & a , const T & b) const {
        return a < b;
    }
};

template<typename K, typename V,typename Comp = Less<K>>
class skip_list {

private:

    struct skip_list_node {

        int level;

        const K key;

        V value;

        skip_list_node** forward;

        skip_list_node() : key{ 0 }, value{ 0 }, level{ 0 }, forward{ 0 } { }

        skip_list_node(K k, V v, int l, skip_list_node* nxt = nullptr) :key(k), value(v), level(l) {
            forward = new skip_list_node* [level + 1];
            for (int i = 0; i <= level; ++i) forward[i] = nxt;
        }

        ~skip_list_node() { delete[] forward; }
    };
    using node = skip_list_node;


    int level;
    int nodeCount;
    static const int max_level = 20;

    node* head, * tail;
    Comp less;

    std::ofstream file_writer;
    std::ifstream file_reader;

private:

    int randomLevel() {

        int k = 1;
        while (rand() % 2) {
            k++;
        }

        k = (k < max_level) ? k : max_level;
        return k;
    }

    node* find(const K& key, node** update) {

        node* p = head;

        for (int i = level; i >= 0; i--) {
            while (p->forward[i] != tail && less(p->forward[i]->key, key)) {
                p = p->forward[i];
            }
            update[i] = p;
        }
        p = p->forward[0];

        return p;
    }

    bool is_valid_string(const std::string& str) {

        if (str.empty()) {
            return false;
        }
        if (str.find(delimiter) == std::string::npos) {
            return false;
        }

        return true;
    }

    void check_string(const std::string& str, std::string* key, std::string* value) {

        if(!is_valid_string(str)) {
            return;
        }
        
        *key = str.substr(0, str.find(delimiter));
        *value = str.substr(str.find(delimiter)+1, str.length());
    }


public:
   
    skip_list(Comp _less = Comp()) : head(new node()), tail(new node()),  less{_less} 
    {
        level = nodeCount = 0;

        head->forward = new node* [max_level + 1];
        for (int i = 0; i <= max_level; i++)
            head->forward[i] = tail;
    }

    ~skip_list() {
        if (file_writer.is_open()) {
            file_writer.close();
        }
        if (file_reader.is_open()) {
            file_reader.close();
        }
        delete head;
    }

    bool insert(const K& key, const V& value) {

        std::lock_guard<std::mutex>locker(mtx);

        node * update[max_level + 1];
        node* p = find(key, update);

        if (p->key == key) {
            // std::cout << "element exist" << endl;
            return false;
        }

        int lv = randomLevel();
        if (lv > level) {
            lv = ++level;
            update[lv] = head;
        }

        node* newNode = new node(key, value, lv);
        for (int i = lv; i >= 0; --i) {
            p = update[i];
            newNode->forward[i] = p->forward[i];
            p->forward[i] = newNode;
        }

        ++nodeCount;
        return 1;
    }

    bool search(const K& key) {

        std::lock_guard<std::mutex>locker(mtx);

        node* update[max_level + 1];
        node* p = find(key, update);

        if (p == tail || p->key != key)return false;
        return  p->value;
    }

    bool erase(const K& key) {
        
        std::lock_guard<std::mutex>locker(mtx);

        node* update[max_level + 1];
        node* p = find(key, update);

        if (p->key != key)return false;

        for (int i = 0; i <= p->level; ++i) {
            update[i]->forward[i] = p->forward[i];
        }
        delete p;

        while (level > 0 && head->forward[level] == tail) 
            --level;
        
        --nodeCount;
        return true;
    }
    
    bool change(const K& key, const V& value) {

        std::lock_guard<std::mutex>locker(mtx);

        node* update[max_level + 1];
        node* p = find(key, update);

        if (p == tail || p->key != key)return false;
        
        p->value = value;
    }

    
    int size() {
        return nodeCount;
    }

    void display_list() {

        std::cout << "\n*****Skip List*****"<<"\n"; 

        for (int i = 0; i <= level; i++) {
            node *n = this->head->forward[i]; 
            std::cout << "Level " << i << ": ";
            while (n != tail) {
                std::cout << n->key << ":" << n->value << ";";
                n = n->forward[i];
            }
            std::cout << std::endl;
        }
    }

    struct Iter {
        node* p;

        Iter() : p(nullptr) {};
        Iter(node* rhs) : p(rhs) {}

        node* operator ->()const { return (p);}
        node& operator *() const { return *p;}

        bool operator == (const Iter& rhs) { return rhs.p == p;}
        bool operator != (const Iter& rhs) {return !(rhs.p == p);}

        void operator ++() {p = p->forward[0];}
        void operator ++(int) { p = p->forward[0]; }
    };

    Iter find(const K& key) {

        node* update[max_level + 1];
        node* p = find(key, update);

        if (p == tail)return tail;
        if (p->key != key)return tail;
        return Iter(p);
    }

    Iter end() {
        return Iter(tail);
    }   

    Iter begin() {
        return Iter(head->forward[0]);
    }


    void dump_file() {

        std::cout << "dump_file-----------------" << std::endl;

        file_writer.open(STORE_FILE, ios::app);
        node *n = this->head->forward[0]; 

        while (n != tail) {
            file_writer << n->key << ":" << n->value << "\n";
            // std::cout << n->key << ":" << n->value << ";\n";
            n = n->forward[0];
        }

        file_writer.flush();
        file_writer.close();

        std::cout << "dump success---------------" << std::endl;
        return;
    }

    void load_file() {

        std::cout << "load_file-----------------" << std::endl;

        file_reader.open(STORE_FILE);

        std::string line;
        std::string* key = new std::string();
        std::string* value = new std::string();

        while (getline(file_reader, line)) {
            check_string(line, key, value);
            if (key->empty() || value->empty()) {
                continue;
            }
            insert(*key, *value);
            // std::cout << "key:" << *key << "value:" << *value << std::endl;
        }
        file_reader.close();

        std:: cout << "loadfile success----------" << std::endl;
    }

};



#endif