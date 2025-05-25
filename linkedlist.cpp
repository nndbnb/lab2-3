#include <iostream>
#include <stdexcept>


template <typename T>
class LinkedList {
public:
    struct Node {
        T     data;
        Node *next;
        explicit Node(T value) : data(value), next(nullptr) {}
    };

    /*-----  Конструкторы  -----*/
    // 1. Инициализация списком элементов
    LinkedList(T *items, int count) : head(nullptr), tail(nullptr), length(0) {
        if (count < 0) throw std::invalid_argument("Count cannot be negative");
        for (int i = 0; i < count; ++i) Append(items[i]);
    }

    // 2. Пустой список
    LinkedList() : head(nullptr), tail(nullptr), length(0) {}

    // 3. Копирующий
    LinkedList(const LinkedList &other) : LinkedList() {
        for (Node *p = other.head; p; p = p->next) Append(p->data);
    }

    /*-----  Деструктор  -----*/
    ~LinkedList() {
        clear();
    }

    /*-----  API  -----*/
    T             GetFirst() const;
    T             GetLast()  const;
    T             Get(int index) const;
    int           GetLength() const { return length; }
    LinkedList   *GetSubList(int startIndex, int endIndex) const;
    void          Prepend(T item);
    void          Append(T item);
    void          InsertAt(T item, int index);
    LinkedList   *Concat(LinkedList *list) const;
    Node         *GetHeadNode() const { return head; }
    Node         *GetTailNode() const { return tail; }

private:
    Node *head;
    Node *tail;
    int   length;

    void clear() {
        while (head) {
            Node *tmp = head;
            head = head->next;
            delete tmp;
        }
        tail = nullptr;
        length = 0;
    }
};

// -----------------------------
//      Реализации методов
// -----------------------------

template <typename T>
void LinkedList<T>::Append(T item) {
    Node *node = new Node(item);
    if (!head) head = tail = node;
    else {
        tail->next = node;
        tail = node;
    }
    ++length;
}

template <typename T>
void LinkedList<T>::Prepend(T item) {
    Node *node = new Node(item);
    node->next = head;
    head = node;
    if (!tail) tail = head;
    ++length;
}

template <typename T>
T LinkedList<T>::GetFirst() const {
    if (!head) throw std::runtime_error("List is empty");
    return head->data;
}

template <typename T>
T LinkedList<T>::GetLast() const {
    if (!tail) throw std::runtime_error("List is empty");
    return tail->data;
}

template <typename T>
T LinkedList<T>::Get(int index) const {
    if (index < 0 || index >= length) throw std::out_of_range("Index out of range");
    Node *p = head;
    while (index--) p = p->next;
    return p->data;
}

template <typename T>
void LinkedList<T>::InsertAt(T item, int index) {
    if (index < 0 || index > length) throw std::out_of_range("Index out of range");
    if (index == 0)      { Prepend(item); return; }
    if (index == length) { Append(item);  return; }

    Node *prev = head;
    for (int i = 1; i < index; ++i) prev = prev->next;
    Node *node = new Node(item);
    node->next = prev->next;
    prev->next = node;
    ++length;
}

template <typename T>
LinkedList<T> *LinkedList<T>::GetSubList(int startIndex, int endIndex) const {
    if (startIndex < 0 || endIndex >= length || startIndex > endIndex)
        throw std::out_of_range("Invalid indices");

    auto *sub = new LinkedList<T>();
    Node *p = head;
    for (int i = 0; i < startIndex; ++i) p = p->next;
    for (int i = startIndex; i <= endIndex; ++i) {
        sub->Append(p->data);
        p = p->next;
    }
    return sub;
}

template <typename T>
LinkedList<T> *LinkedList<T>::Concat(LinkedList<T> *other) const {
    auto *result = new LinkedList<T>(*this);
    for (Node *p = other->head; p; p = p->next) result->Append(p->data);
    return result;
}

// int main() {
//     int arr[] = {1,2,3};
//     LinkedList<int> a(arr, 3);
//     LinkedList<int> b(a);
//     b.Append(4);
//     auto *sub = b.GetSubList(1,2);
//     auto *c = a.Concat(sub);
//     std::cout << c->GetLength() << std::endl; // 5
//     delete sub;
//     delete c;
//     return 0;
// }

