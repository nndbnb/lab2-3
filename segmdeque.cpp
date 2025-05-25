#include <iostream>
#include <functional>
#include <stdexcept>
#include <algorithm>
#include "sequence.cpp"

template <typename T>
class SegmentedDeque : public MutableSequence<T>
{
private:
    struct Segment
    {
        DynamicArray<T> data;
        Segment *next;
        Segment *prev;

        Segment(int capacity) : data(), next(nullptr), prev(nullptr)
        {
            data.Reserve(capacity);
        }
    };

    Segment *head;
    Segment *tail;
    int segmentCapacity;
    int totalSize;

    void ensureCapacity()
    {
        if (totalSize == 0)
        {
            head = tail = new Segment(segmentCapacity);
            return;
        }

        if (tail->data.GetSize() < segmentCapacity)
        {
            return;
        }

        Segment *newSegment = new Segment(segmentCapacity);
        tail->next = newSegment;
        newSegment->prev = tail;
        tail = newSegment;
    }

    void ensureCapacityFront()
    {
        if (totalSize == 0)
        {
            head = tail = new Segment(segmentCapacity);
            return;
        }

        if (head->data.GetSize() < segmentCapacity)
        {
            return;
        }

        Segment *newSegment = new Segment(segmentCapacity);
        head->prev = newSegment;
        newSegment->next = head;
        head = newSegment;
    }

    std::pair<Segment *, int> findSegmentAndIndex(int index) const
    {
        if (index < 0 || index >= totalSize)
        {
            throw std::out_of_range("Index out of range");
        }

        Segment *current = head;
        int remaining = index;

        while (current != nullptr)
        {
            int currentSize = current->data.GetSize();
            if (remaining < currentSize)
            {
                return {current, remaining};
            }
            remaining -= currentSize;
            current = current->next;
        }

        throw std::out_of_range("Index out of range");
    }

public:
    SegmentedDeque(int segmentSize = 4) : head(nullptr), tail(nullptr), segmentCapacity(segmentSize), totalSize(0)
    {
        if (segmentSize <= 0)
        {
            throw std::invalid_argument("Segment size must be positive");
        }
    }

    SegmentedDeque(const SegmentedDeque<T> &other) : head(nullptr), tail(nullptr), segmentCapacity(other.segmentCapacity), totalSize(0)
    {
        Segment *current = other.head;
        while (current != nullptr)
        {
            for (int i = 0; i < current->data.GetSize(); ++i)
            {
                AppendInPlace(current->data.Get(i));
            }
            current = current->next;
        }
    }

    ~SegmentedDeque()
    {
        Segment *current = head;
        while (current != nullptr)
        {
            Segment *next = current->next;
            delete current;
            current = next;
        }
    }

    T GetFirst() const override
    {
        if (totalSize == 0)
        {
            throw std::out_of_range("Deque is empty");
        }
        return head->data.Get(0);
    }

    T GetLast() const override
    {
        if (totalSize == 0)
        {
            throw std::out_of_range("Deque is empty");
        }
        return tail->data.Get(tail->data.GetSize() - 1);
    }

    T Get(int index) const override
    {
        auto [segment, idx] = findSegmentAndIndex(index);
        return segment->data.Get(idx);
    }

    Sequence<T> *GetSubsequence(int startIndex, int endIndex) const override
    {
        if (startIndex < 0 || endIndex >= totalSize || startIndex > endIndex)
        {
            throw std::out_of_range("Invalid indices");
        }

        SegmentedDeque<T> *subseq = new SegmentedDeque<T>(segmentCapacity);
        for (int i = startIndex; i <= endIndex; ++i)
        {
            subseq->AppendInPlace(Get(i));
        }
        return subseq;
    }

    int GetLength() const override
    {
        return totalSize;
    }

    void AppendInPlace(T item) override
    {
        ensureCapacity();
        tail->data.Append(item);
        totalSize++;
    }

    void PrependInPlace(T item) override
    {
        ensureCapacityFront();
        if (head->data.GetSize() == 0)
        {
            head->data.Append(item);
        }
        else
        {
            DynamicArray<T> newData;
            newData.Append(item);
            for (int i = 0; i < head->data.GetSize(); ++i)
            {
                newData.Append(head->data.Get(i));
            }
            head->data = newData;
        }
        totalSize++;
    }

    void InsertAtInPlace(T item, int index) override
    {
        if (index == 0)
        {
            PrependInPlace(item);
            return;
        }
        if (index == totalSize)
        {
            AppendInPlace(item);
            return;
        }

        auto [segment, idx] = findSegmentAndIndex(index);

        // Если в текущем сегменте есть место
        if (segment->data.GetSize() < segmentCapacity)
        {
            DynamicArray<T> newData;
            for (int i = 0; i < idx; ++i)
            {
                newData.Append(segment->data.Get(i));
            }
            newData.Append(item);
            for (int i = idx; i < segment->data.GetSize(); ++i)
            {
                newData.Append(segment->data.Get(i));
            }
            segment->data = newData;
        }
        else
        {
            // Нужно разделить сегмент
            Segment *newSegment = new Segment(segmentCapacity);
            newSegment->next = segment->next;
            if (segment->next)
            {
                segment->next->prev = newSegment;
            }
            else
            {
                tail = newSegment;
            }
            segment->next = newSegment;
            newSegment->prev = segment;

            // Переносим часть данных в новый сегмент
            DynamicArray<T> newData1, newData2;
            for (int i = 0; i < idx; ++i)
            {
                newData1.Append(segment->data.Get(i));
            }
            newData1.Append(item);
            for (int i = idx; i < segment->data.GetSize(); ++i)
            {
                newData2.Append(segment->data.Get(i));
            }

            segment->data = newData1;
            newSegment->data = newData2;
        }
        totalSize++;
    }

    Sequence<T> *Append(T item) const override
    {
        SegmentedDeque<T> *newDeque = new SegmentedDeque<T>(*this);
        newDeque->AppendInPlace(item);
        return newDeque;
    }

    Sequence<T> *Prepend(T item) const override
    {
        SegmentedDeque<T> *newDeque = new SegmentedDeque<T>(*this);
        newDeque->PrependInPlace(item);
        return newDeque;
    }

    Sequence<T> *InsertAt(T item, int index) const override
    {
        SegmentedDeque<T> *newDeque = new SegmentedDeque<T>(*this);
        newDeque->InsertAtInPlace(item, index);
        return newDeque;
    }

    Sequence<T> *Concat(Sequence<T> *other) const override
    {
        SegmentedDeque<T> *newDeque = new SegmentedDeque<T>(*this);
        for (int i = 0; i < other->GetLength(); ++i)
        {
            newDeque->AppendInPlace(other->Get(i));
        }
        return newDeque;
    }

    // Sorting methods
    void SortInPlace(const std::function<bool(const T &, const T &)> &comparator = [](const T &a, const T &b)
                     { return a < b; })
    {
        // Collect all elements into a temporary array
        DynamicArray<T> tempArray;
        Segment *current = head;
        while (current != nullptr)
        {
            for (int i = 0; i < current->data.GetSize(); ++i)
            {
                tempArray.Append(current->data.Get(i));
            }
            current = current->next;
        }

        // Sort the temporary array
        tempArray.SortInPlace(comparator);

        // Rebuild the deque from the sorted array
        Clear();
        for (int i = 0; i < tempArray.GetSize(); ++i)
        {
            AppendInPlace(tempArray.Get(i));
        }
    }

    Sequence<T> *Sort(const std::function<bool(const T &, const T &)> &comparator = [](const T &a, const T &b)
                      { return a < b; }) const
    {
        SegmentedDeque<T> *newDeque = new SegmentedDeque<T>(*this);
        newDeque->SortInPlace(comparator);
        return newDeque;
    }

    // Applies a function to each element and returns a new deque
    Sequence<T> *Map(const std::function<T(const T &)> &mapper) const
    {
        SegmentedDeque<T> *newDeque = new SegmentedDeque<T>(segmentCapacity);
        Segment *current = head;
        while (current != nullptr)
        {
            for (int i = 0; i < current->data.GetSize(); ++i)
            {
                newDeque->AppendInPlace(mapper(current->data.Get(i)));
            }
            current = current->next;
        }
        return newDeque;
    }

    // Filters elements based on a predicate and returns a new deque
    Sequence<T> *Where(const std::function<bool(const T &)> &predicate) const
    {
        SegmentedDeque<T> *newDeque = new SegmentedDeque<T>(segmentCapacity);
        Segment *current = head;
        while (current != nullptr)
        {
            for (int i = 0; i < current->data.GetSize(); ++i)
            {
                T item = current->data.Get(i);
                if (predicate(item))
                {
                    newDeque->AppendInPlace(item);
                }
            }
            current = current->next;
        }
        return newDeque;
    }

    // Reduces the sequence to a single value using the given function
    T Reduce(const std::function<T(const T &, const T &)> &reducer, T initial) const
    {
        T result = initial;
        Segment *current = head;
        while (current != nullptr)
        {
            for (int i = 0; i < current->data.GetSize(); ++i)
            {
                result = reducer(result, current->data.Get(i));
            }
            current = current->next;
        }
        return result;
    }

    // Checks if the deque contains a given subsequence
    bool ContainsSubsequence(const Sequence<T> &subseq) const
    {
        if (subseq.GetLength() == 0)
        {
            return true;
        }
        if (subseq.GetLength() > totalSize)
        {
            return false;
        }

        // Convert subsequence to array for easier access
        DynamicArray<T> subseqArray;
        for (int i = 0; i < subseq.GetLength(); ++i)
        {
            subseqArray.Append(subseq.Get(i));
        }

        // Look for the subsequence in our deque
        for (int i = 0; i <= totalSize - subseq.GetLength(); ++i)
        {
            bool match = true;
            for (int j = 0; j < subseq.GetLength(); ++j)
            {
                if (Get(i + j) != subseqArray.Get(j))
                {
                    match = false;
                    break;
                }
            }
            if (match)
            {
                return true;
            }
        }
        return false;
    }

    void Clear()
    {
        Segment *current = head;
        while (current != nullptr)
        {
            Segment *next = current->next;
            delete current;
            current = next;
        }
        head = tail = nullptr;
        totalSize = 0;
    }

    void PrintDebugInfo() const
    {
        std::cout << "SegmentedDeque (size=" << totalSize << ", segments=";
        int segmentCount = 0;
        Segment *current = head;
        while (current)
        {
            segmentCount++;
            current = current->next;
        }
        std::cout << segmentCount << "):" << std::endl;

        current = head;
        int segNum = 0;
        while (current)
        {
            std::cout << "  Segment " << segNum++ << " (size=" << current->data.GetSize()
                      << ", capacity=" << current->data.GetCapacity() << "): ";
            for (int i = 0; i < current->data.GetSize(); ++i)
            {
                std::cout << current->data.Get(i) << " ";
            }
            std::cout << std::endl;
            current = current->next;
        }
    }
};

int main()
{
    try
    {
        std::cout << "=== Проверка SegmentedDeque ===\n";

        SegmentedDeque<int> dq(3); // размер сегмента: 3

        std::cout << "\nДобавление значений 1–5:\n";
        for (int val = 1; val <= 5; ++val)
        {
            dq.AppendInPlace(val);
            dq.PrintDebugInfo();
        }

        std::cout << "\nОбращение к элементам:\n";
        for (int i = 0; i < dq.GetLength(); ++i)
            std::cout << "[" << i << "] = " << dq.Get(i) << '\n';

        std::cout << "\nПервый: " << dq.GetFirst() << ", Последний: " << dq.GetLast() << '\n';

        std::cout << "\nДобавление в начало: 0\n";
        dq.PrependInPlace(0);
        dq.PrintDebugInfo();

        std::cout << "\nВставка 10 на позицию 3\n";
        dq.InsertAtInPlace(10, 3);
        dq.PrintDebugInfo();

        std::cout << "\nИзвлечение подотрезка (1, 3):\n";
        auto *sub = dq.GetSubsequence(1, 3);
        for (int i = 0; i < sub->GetLength(); ++i)
            std::cout << sub->Get(i) << ' ';
        std::cout << '\n';
        delete sub;

        std::cout << "\nСоздание второго дека (20–22):\n";
        SegmentedDeque<int> dq2(2);
        for (int val = 20; val <= 22; ++val)
            dq2.AppendInPlace(val);
        dq2.PrintDebugInfo();

        std::cout << "\nОбъединение двух деков:\n";
        auto *joined = dq.Concat(&dq2);
        for (int i = 0; i < joined->GetLength(); ++i)
            std::cout << joined->Get(i) << ' ';
        std::cout << '\n';
        delete joined;

        std::cout << "\nНеизменяемые операции:\n";
        auto *newDq = dq.Append(100);
        std::cout << "Хвост оригинала: " << dq.GetLast() << ", нового: " << newDq->GetLast() << '\n';
        delete newDq;

        std::cout << "\nПроверка исключений:\n";
        try
        {
            std::cout << dq.Get(-1) << '\n';
        }
        catch (const std::out_of_range &e)
        {
            std::cout << "Исключение: " << e.what() << '\n';
        }

        SegmentedDeque<int> d(3);
        for (int i = 0; i < 10; ++i)
            d.AppendInPlace(9 - i);

        std::cout << "\n=== Дополнительные методы ===\n";
        std::cout << "\nИсходный дек:\n";
        d.PrintDebugInfo();

        std::cout << "\nСортировка по возрастанию:\n";
        d.SortInPlace();
        d.PrintDebugInfo();

        std::cout << "\nСортировка по убыванию:\n";
        d.SortInPlace([](const int &a, const int &b) { return a > b; });
        d.PrintDebugInfo();

        std::cout << "\nВозведение в квадрат:\n";
        auto *squared = d.Map([](int x) { return x * x; });
        for (int i = 0; i < squared->GetLength(); ++i)
            std::cout << squared->Get(i) << ' ';
        std::cout << '\n';
        delete squared;

        std::cout << "\nФильтр (чётные):\n";
        auto *evens = d.Where([](int x) { return x % 2 == 0; });
        for (int i = 0; i < evens->GetLength(); ++i)
            std::cout << evens->Get(i) << ' ';
        std::cout << '\n';
        delete evens;

        std::cout << "\nСуммирование:\n";
        int total = d.Reduce([](int a, int b) { return a + b; }, 0);
        std::cout << "Сумма: " << total << '\n';

        std::cout << "Максимум:\n";
        int maximum = d.Reduce([](int a, int b) { return (a > b) ? a : b; }, d.Get(0));
        std::cout << "Макс: " << maximum << '\n';

        std::cout << "\nПоиск подпоследовательностей:\n";
        ArraySequence<int> sub1, sub2, sub3;
        sub1.AppendInPlace(9); sub1.AppendInPlace(8); sub1.AppendInPlace(7);
        sub2.AppendInPlace(5); sub2.AppendInPlace(4); sub2.AppendInPlace(3);
        sub3.AppendInPlace(1); sub3.AppendInPlace(2); sub3.AppendInPlace(3);

        std::cout << "[9,8,7]? " << (d.ContainsSubsequence(sub1) ? "Да" : "Нет") << '\n';
        std::cout << "[5,4,3]? " << (d.ContainsSubsequence(sub2) ? "Да" : "Нет") << '\n';
        std::cout << "[1,2,3]? " << (d.ContainsSubsequence(sub3) ? "Да" : "Нет") << '\n';

        std::cout << "\nГраничные случаи:\n";

        ArraySequence<int> empty;
        std::cout << "Пустая подпоследовательность? " << (d.ContainsSubsequence(empty) ? "Да" : "Нет") << '\n';

        ArraySequence<int> tooLong;
        for (int i = 0; i < 20; ++i)
            tooLong.AppendInPlace(i);
        std::cout << "Подпоследовательность длиннее дека? " << (d.ContainsSubsequence(tooLong) ? "Да" : "Нет") << '\n';

        std::cout << "\nВсе проверки завершены.\n";
    }
    catch (const std::exception &e)
    {
        std::cerr << "Ошибка: " << e.what() << '\n';
        return 1;
    }

    return 0;
}
