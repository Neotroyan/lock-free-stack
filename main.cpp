#include <iostream>
#include <vector>
#include <atomic>

using namespace std;

struct Node {
    int data;
    Node* next;
    atomic<int> ref_count;

    Node(int data) : data(data), next(nullptr), ref_count(0) {}
};

struct Stack {
    atomic<Node*> head;

    Stack() : head(nullptr) {}

    void push(int data) {
        Node* newNode = new Node(data);
        newNode->ref_count.store(1);
        while (true) {
            newNode->next = head.load();
            Node* old_head = head;
            if (head.compare_exchange_weak(old_head, newNode)) {
                if (newNode->next) {
                    newNode->next->ref_count.fetch_add(1);
                }
                break;
            }
            
        }
    }

    int pop() {
        Node* old_head;
        Node* new_head;
        while (true) {
            old_head = head.load();
            if (old_head == NULL) {
                continue;
            }
            vector<Node*> hazardPointers{ old_head };
            new_head = old_head->next;
            if (head.compare_exchange_weak(old_head, new_head)) {
                int poppedValue = old_head->data;

                if (old_head->ref_count.fetch_sub(1) == 1) {
                    delete old_head;
                }
                return poppedValue;

            }
        }
    }

    ~Stack() {
        Node* temp;
        while ((temp = head.load()) != nullptr) {
            head.store(temp->next);
            delete temp;
        }
    }


};

int main() {
    Stack stack;

    stack.push(10);
    stack.push(20);
    stack.push(30);

    std::cout << "Popped: " << stack.pop() << std::endl;
    std::cout << "Popped: " << stack.pop() << std::endl;

    return 0;
}