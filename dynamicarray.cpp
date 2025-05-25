#include <algorithm>
#include <functional>
#include <stdexcept>


template <typename T>
class DynamicArray {
private:
    T  *data      = nullptr;
    int capacity  = 0;
    int size      = 0;

    /* reallocates underlying buffer, preserving existing elements */
    void reallocate(int new_capacity) {
        if (new_capacity < size) new_capacity = size;
        if (new_capacity == capacity) return;

        T *tmp = new T[new_capacity];
        for (int i = 0; i < size; ++i) tmp[i] = std::move(data[i]);
        delete[] data;
        data     = tmp;
        capacity = new_capacity;
    }

public:
    /* ---------- ctors / dtor ---------- */
    DynamicArray() : data(new T[1]), capacity(1), size(0) {}

    explicit DynamicArray(int initial_size)
        : capacity(initial_size > 0 ? initial_size : 1),
          size(initial_size)
    {
        data = new T[capacity];
        std::fill_n(data, size, T());
    }

    DynamicArray(const DynamicArray &other)
        : data(new T[other.capacity]), capacity(other.capacity), size(other.size)
    {
        std::copy(other.data, other.data + size, data);
    }

    DynamicArray &operator=(const DynamicArray &other) {
        if (this == &other) return *this;
        delete[] data;
        capacity = other.capacity;
        size     = other.size;
        data     = new T[capacity];
        std::copy(other.data, other.data + size, data);
        return *this;
    }

    ~DynamicArray() { delete[] data; }

    /* ---------- capacity helpers ---------- */
    void Reserve(int new_capacity) { if (new_capacity > capacity) reallocate(new_capacity); }

    void Resize(int new_size) {
        if (new_size < 0) throw std::invalid_argument("Size cannot be negative");
        if (new_size > capacity) reallocate(std::max(new_size, capacity * 2));
        size = new_size;
    }

    /* ---------- element access ---------- */
    T Get(int index) const {
        if (index < 0 || index >= size) throw std::out_of_range("Index out of range");
        return data[index];
    }

    void Set(int index, T value) {
        if (index < 0 || index >= size) throw std::out_of_range("Index out of range");
        data[index] = value;
    }

    /* ---------- modifiers ---------- */
    void Append(T item) {
        if (size == capacity) reallocate(capacity ? capacity * 2 : 1);
        data[size++] = item;
    }

    void SortInPlace(const std::function<bool(const T&, const T&)> &comp = [](const T &a, const T &b){ return a < b; }) {
        // naive bubble sort kept for educational purposes
        for (int i = 0; i < size; ++i)
            for (int j = 0; j < size - i - 1; ++j)
                if (!comp(data[j], data[j + 1])) std::swap(data[j], data[j + 1]);
    }

    /* ---------- getters ---------- */
    int GetSize() const { return size; }
    int GetCapacity() const { return capacity; }
};
