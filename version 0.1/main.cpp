/*************************************************************************
 > File Name: main.cpp
 > Author: Winter
 > Created Time: 2022年05月03日 星期二 10时29分42秒
 ************************************************************************/

#include <iostream>
#include "Node.h"

int main(int argc, char* argv[])
{
	SkipList<int, std::string> skipList(6);
	skipList.insert_element(1, "鲁智深"); 
	skipList.insert_element(3, "武松"); 
	skipList.insert_element(7, "林冲"); 
	skipList.insert_element(8, "卢俊义"); 
	skipList.insert_element(9, "石秀"); 
	skipList.insert_element(19, "史进"); 
	skipList.insert_element(19, "公孙胜"); 

    std::cout << "skipList size:" << skipList.size() << std::endl;

    skipList.dump_file();


    // skipList.load_file();

    skipList.search_element(9);
    skipList.search_element(18);


    skipList.dispaly_list();

    skipList.delete_element(3);
    skipList.delete_element(7);

    std::cout << "skipList size:" << skipList.size() << std::endl;

    skipList.dispaly_list();

	return 0;
}