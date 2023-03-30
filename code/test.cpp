/*
 * @Description: 测试跳表功能
 * @Autor: KylinLzw
 * @LastEditTime: 2023-03-30 09:24:03
 */

#include "skiplist.h"

int main()
{

    {
        // 默认小于号
        skip_list<int, int>list;
        list.insert(1, 1);

        list.insert(2, 2);
        list.insert(2, 3);

        list.insert(3, 3);
        list.insert(4, 1);

        for (auto it = list.begin(); it != list.end();it++) {
            cout << it->key << " " << it->value << endl;
        }
       
    }

    cout << "==================================" << endl;
    
    {
        //使用仿函数
        struct cmp {
            bool operator()(int a, int b) {
                return a > b;
            }
        };
        skip_list < int, int, cmp> list{};

        for (int i = 1; i <= 10; i++)list.insert(i, i);
        for (auto it = list.find(5); it != list.end(); it++) {
            cout << it->key << " " << it->value << endl;
        }

        list.dump_file();

    }

    cout << "==================================" << endl;


    {
        //使用lambda
        auto cmp = [](const string& a, const string& b) {return a.length() < b.length(); };
        skip_list < string, string, decltype(cmp)> list(cmp);

        
        list.insert("lzw", "111");
        list.insert("lzw1", "222");
        list.insert("lzw11", "333");
        list.insert("lzw111", "444");
        list.insert("lzw1111", "555");
        list.insert("lzw11111", "666");


        for (auto it = list.begin(); it != list.end(); it++) {
            cout << it->key << " " << it->value << endl;
        }

        list.dump_file();
        list.load_file();

        list.display_list();

        std::cout << list.size() << endl;

    }
    
    
}
