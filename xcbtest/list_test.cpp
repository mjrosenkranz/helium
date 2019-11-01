#include <iostream>
#include <list>

using namespace std;

list<int> intlist;

void printallint(list<int> listy);

void printallint(list<int> listy) {
    list<int>::iterator it;
    for (it = listy.begin(); it != listy.cend(); it++) {
        fprintf(stderr, "%d\n", *it);
    }
}

template <typename T>
void printall(const list<T> &listy) {
    for (auto it = listy.cbegin(); it != listy.end(); it++) {
        fprintf(stderr, "%d\n\n", *it);
    }
}


int main() {
    intlist.push_front(1);
    intlist.push_back(2);
    intlist.push_back(3);

    printall(intlist);

    intlist.remove(3);
    intlist.push_front(3);
    printall(intlist);


    return 0;
}
