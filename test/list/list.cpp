// list.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <atlcoll.h>


int main()
{
	CAtlList<int> list;
	CAtlList<int> list2;

	for (int i = 0; i < 10; i++) {
		list.AddTail(i);
	}	

	POSITION pos = list.GetHeadPosition();
	while (pos) {
		int s = list.GetNext(pos);
		list2.AddTail(s);
	}

	list.RemoveAll();
	for (int i = 10; i < 20; i++) {
		list.AddTail(i);
	}

	memcpy(&list2, &list, sizeof(list));
	
	/*POSITION pos1 = list.GetHeadPosition();
	while (pos1) {
		int s1 = list.GetNext(pos1);
		printf("list1= %i\n", s1);
	}*/

	POSITION pos2 = list2.GetHeadPosition();
	while (pos2) {
		int s2 = list2.GetNext(pos2);
		printf("list2= %i\n", s2);
	}

    return 0;
}

