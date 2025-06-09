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

    // Метод для слияния соседних неполных сегментов
    void mergeSegments()
    {
        Segment *current = head;
        while (current && current->next)
        {
            if (current->data.GetSize() + current->next->data.GetSize() <= segmentCapacity)
            {
                // Объединяем сегменты
                for (int i = 0; i < current->next->data.GetSize(); ++i)
                {
                    current->data.Append(current->next->data.Get(i));
                }

                Segment *toDelete = current->next;
                current->next = toDelete->next;
                if (toDelete->next)
                {
                    toDelete->next->prev = current;
                }
                else
                {
                    tail = current;
                }
                delete toDelete;
            }
            else
            {
                current = current->next;
            }
        }
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

    // Оператор присваивания
    SegmentedDeque<T> &operator=(const SegmentedDeque<T> &other)
    {
        if (this == &other)
            return *this;

        Clear();
        segmentCapacity = other.segmentCapacity;

        Segment *current = other.head;
        while (current != nullptr)
        {
            for (int i = 0; i < current->data.GetSize(); ++i)
            {
                AppendInPlace(current->data.Get(i));
            }
            current = current->next;
        }
        return *this;
    }

    ~SegmentedDeque()
    {
        Clear();
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

    // Проверка на пустоту
    bool IsEmpty() const
    {
        return totalSize == 0;
    }

    void AppendInPlace(T item) override
    {
        ensureCapacity();
        tail->data.Append(item);
        totalSize++;
    }

    // Улучшенная версия PrependInPlace
    void PrependInPlace(T item) override
    {
        if (totalSize == 0)
        {
            ensureCapacity();
            head->data.Append(item);
            totalSize++;
            return;
        }

        // Если в первом сегменте есть место, просто вставляем в начало
        if (head->data.GetSize() < segmentCapacity)
        {
            // Более эффективная вставка в начало
            head->data.Resize(head->data.GetSize() + 1);
            for (int i = head->data.GetSize() - 1; i > 0; --i)
            {
                head->data.Set(i, head->data.Get(i - 1));
            }
            head->data.Set(0, item);
        }
        else
        {
            // Создаем новый сегмент в начале
            Segment *newSegment = new Segment(segmentCapacity);
            newSegment->data.Append(item);
            newSegment->next = head;
            if (head)
            {
                head->prev = newSegment;
            }
            head = newSegment;
            if (!tail)
            {
                tail = head;
            }
        }
        totalSize++;
    }

    void InsertAtInPlace(T item, int index) override
    {
        if (index < 0 || index > totalSize)
        {
            throw std::out_of_range("Index out of range");
        }

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

        if (segment->data.GetSize() < segmentCapacity)
        {
            // Есть место в сегменте - просто сдвигаем элементы
            segment->data.Resize(segment->data.GetSize() + 1);
            for (int i = segment->data.GetSize() - 1; i > idx; --i)
            {
                segment->data.Set(i, segment->data.Get(i - 1));
            }
            segment->data.Set(idx, item);
        }
        else
        {
            // Сегмент полон - разделяем его
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

            // Определяем середину для разделения
            int mid = segmentCapacity / 2;
            
            if (idx <= mid)
            {
                // Вставляем в первую половину
                for (int i = mid; i < segmentCapacity; ++i)
                {
                    newSegment->data.Append(segment->data.Get(i));
                }
                segment->data.Resize(mid);
                
                // Вставляем элемент
                segment->data.Resize(segment->data.GetSize() + 1);
                for (int i = segment->data.GetSize() - 1; i > idx; --i)
                {
                    segment->data.Set(i, segment->data.Get(i - 1));
                }
                segment->data.Set(idx, item);
            }
            else
            {
                // Вставляем во вторую половину
                for (int i = mid; i < idx; ++i)
                {
                    newSegment->data.Append(segment->data.Get(i));
                }
                newSegment->data.Append(item);
                for (int i = idx; i < segmentCapacity; ++i)
                {
                    newSegment->data.Append(segment->data.Get(i));
                }
                segment->data.Resize(mid);
            }
        }
        totalSize++;
    }

    // Удаление с начала
    T PopFront()
    {
        if (totalSize == 0)
        {
            throw std::out_of_range("Deque is empty");
        }

        T result = head->data.Get(0);

        // Сдвигаем элементы в первом сегменте
        for (int i = 0; i < head->data.GetSize() - 1; ++i)
        {
            head->data.Set(i, head->data.Get(i + 1));
        }
        head->data.Resize(head->data.GetSize() - 1);
        totalSize--;

        // Если сегмент стал пустым и есть следующий
        if (head->data.GetSize() == 0 && head->next)
        {
            Segment *oldHead = head;
            head = head->next;
            head->prev = nullptr;
            delete oldHead;
        }

        // Если дек стал пустым
        if (totalSize == 0)
        {
            head = tail = nullptr;
        }

        return result;
    }

    // Удаление с конца
    T PopBack()
    {
        if (totalSize == 0)
        {
            throw std::out_of_range("Deque is empty");
        }

        T result = tail->data.Get(tail->data.GetSize() - 1);
        tail->data.Resize(tail->data.GetSize() - 1);
        totalSize--;

        // Если сегмент стал пустым и есть предыдущий
        if (tail->data.GetSize() == 0 && tail->prev)
        {
            Segment *oldTail = tail;
            tail = tail->prev;
            tail->next = nullptr;
            delete oldTail;
        }

        // Если дек стал пустым
        if (totalSize == 0)
        {
            head = tail = nullptr;
        }

        return result;
    }

    // Удаление по индексу
    T RemoveAt(int index)
    {
        if (index < 0 || index >= totalSize)
        {
            throw std::out_of_range("Index out of range");
        }

        if (index == 0)
            return PopFront();
        if (index == totalSize - 1)
            return PopBack();

        auto [segment, idx] = findSegmentAndIndex(index);
        T result = segment->data.Get(idx);

        // Сдвигаем элементы в сегменте
        for (int i = idx; i < segment->data.GetSize() - 1; ++i)
        {
            segment->data.Set(i, segment->data.Get(i + 1));
        }
        segment->data.Resize(segment->data.GetSize() - 1);
        totalSize--;

        // Если сегмент стал пустым, удаляем его
        if (segment->data.GetSize() == 0)
        {
            if (segment->prev)
            {
                segment->prev->next = segment->next;
            }
            else
            {
                head = segment->next;
            }

            if (segment->next)
            {
                segment->next->prev = segment->prev;
            }
            else
            {
                tail = segment->prev;
            }

            delete segment;
        }

        return result;
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

    void SortInPlace(const std::function<bool(const T &, const T &)> &comparator = [](const T &a, const T &b)
                     { return a < b; })
    {
        // Более эффективная сортировка без полного копирования
        if (totalSize <= 1) return;
        
        // Собираем все элементы
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

        // Сортируем
        tempArray.SortInPlace(comparator);

        // Распределяем обратно по сегментам
        int tempIndex = 0;
        current = head;
        while (current != nullptr && tempIndex < tempArray.GetSize())
        {
            int segmentSize = current->data.GetSize();
            current->data.Resize(0); // Очищаем сегмент
            
            for (int i = 0; i < segmentSize && tempIndex < tempArray.GetSize(); ++i)
            {
                current->data.Append(tempArray.Get(tempIndex++));
            }
            current = current->next;
        }
    }

    Sequence<T> *Sort(const std::function<bool(const T &, const T &)> &comparator = [](const T &a, const T &b)
                      { return a < b; }) const
    {
        SegmentedDeque<T> *newDeque = new SegmentedDeque<T>(*this);
        newDeque->SortInPlace(comparator);
        return newDeque;
    }

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

        for (int i = 0; i <= totalSize - subseq.GetLength(); ++i)
        {
            bool match = true;
            for (int j = 0; j < subseq.GetLength(); ++j)
            {
                if (Get(i + j) != subseq.Get(j))
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

    void Optimize()
    {
        mergeSegments();
    }

    void Reserve(int expectedSize)
    {
        int segmentsNeeded = (expectedSize + segmentCapacity - 1) / segmentCapacity;
        int currentSegments = 0;
        
        Segment *current = head;
        while (current)
        {
            currentSegments++;
            current = current->next;
        }
        
        for (int i = currentSegments; i < segmentsNeeded; ++i)
        {
            ensureCapacity();
        }
    }

    void PrintDebugInfo() const
    {
        std::cout << "SegmentedDeque (size=" << totalSize << ", capacity=" << segmentCapacity << ", segments=";
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
        std::cout << "=== Проверка улучшенного SegmentedDeque ===\n";

        SegmentedDeque<int> dq(3); // размер сегмента: 3

        std::cout << "\nДобавление значений 1–5:\n";
        for (int val = 1; val <= 5; ++val)
        {
            dq.AppendInPlace(val);
        }
        dq.PrintDebugInfo();

        std::cout << "\nТестирование новых методов:\n";
        
        // Тест PopFront
        std::cout << "PopFront: " << dq.PopFront() << std::endl;
        dq.PrintDebugInfo();
        
        // Тест PopBack
        std::cout << "PopBack: " << dq.PopBack() << std::endl;
        dq.PrintDebugInfo();
        
        // Тест RemoveAt
        std::cout << "RemoveAt(1): " << dq.RemoveAt(1) << std::endl;
        dq.PrintDebugInfo();
        
        // Тест IsEmpty
        std::cout << "IsEmpty: " << (dq.IsEmpty() ? "true" : "false") << std::endl;
        
        // Заполняем заново для остальных тестов
        dq.Clear();
        for (int i = 1; i <= 10; ++i)
        {
            dq.AppendInPlace(i);
        }
        
        std::cout << "\nПосле заполнения 1-10:\n";
        dq.PrintDebugInfo();
        
        // Тест Reserve
        std::cout << "\nТест Reserve(20):\n";
        dq.Reserve(20);
        dq.PrintDebugInfo();
        
        // Тест оператора присваивания
        std::cout << "\nТест оператора присваивания:\n";
        SegmentedDeque<int> dq2(2);
        dq2 = dq;
        std::cout << "Копия:\n";
        dq2.PrintDebugInfo();
        
        // Тест оптимизации
        std::cout << "\nТест оптимизации (удаляем несколько элементов):\n";
        dq.RemoveAt(2);
        dq.RemoveAt(4);
        dq.RemoveAt(6);
        std::cout << "До оптимизации:\n";
        dq.PrintDebugInfo();
        
        dq.Optimize();
        std::cout << "После оптимизации:\n";
        dq.PrintDebugInfo();

        std::cout << "\n=== Все тесты завершены успешно ===\n";
    }
    catch (const std::exception &e)
    {
        std::cerr << "Ошибка: " << e.what() << '\n';
        return 1;
    }

    return 0;
}