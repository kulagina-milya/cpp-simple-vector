// вставьте сюда ваш код для класса SimpleVector
// внесите необходимые изменения для поддержки move-семантики
#pragma once

#include <cassert>
#include <initializer_list>
#include "array_ptr.h"
#include <stdexcept>
#include <algorithm>
#include <iostream>
#include <iterator>

using namespace std;

class ReserveProxyObj {
    public:
    ReserveProxyObj(size_t capacity_to_reserve) :
        capacity_(capacity_to_reserve) {}
    size_t capacity_;
};

ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
}

template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;

    explicit SimpleVector(size_t size) :
        SimpleVector(size, std::move(Type{})) {}

    SimpleVector(size_t size, const Type& value) :
        items_(size), 
        size_(size), 
        capacity_(size) {
            std::fill(begin(), end(), value);
    }

    SimpleVector(std::initializer_list<Type> init) :
        items_(init.size()), 
        size_(init.size()), 
        capacity_(init.size()) {
            std::copy(std::make_move_iterator(init.begin()), std::make_move_iterator(init.end()), begin());
    }

    SimpleVector(ReserveProxyObj capacity_to_reserve) {
        Reserve(capacity_to_reserve.capacity_);
    }

    size_t GetSize() const noexcept {
        return size_;
    }

    size_t GetCapacity() const noexcept {
        return capacity_;
    }

    bool IsEmpty() const noexcept {
        return !size_;
    }

    Type& operator[](size_t index) noexcept {
        return items_[index];
    }

    const Type& operator[](size_t index) const noexcept {
        return items_[index];
    }

    Type& At(size_t index) {
        if (index >= size_) {
            throw std::out_of_range("index >= size_");
        }
    return items_[index];
    }

    const Type& At(size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("index >= size_");
        }
    return items_[index];
    }

    void Clear() noexcept {
        size_ = 0;
    }
        
    void Resize(size_t new_size) {
        if (new_size <= size_) {
            size_ = new_size;
        }
        else if ((new_size > size_) && (new_size < capacity_)) {
            size_ = new_size;
            std::generate(begin() + new_size , end(), [](){
                return std::move(Type{});
            });
        }
        else {
            auto s = std::max(new_size, capacity_ * 2);
            Reserve(s);
            std::generate(begin() + size_, begin() + new_size, [](){
                return std::move(Type{});
            });
            size_ = new_size;
            capacity_ = s;
        }
    }        

    SimpleVector(const SimpleVector& other) :
    items_(other.size_),
    size_(other.size_),
    capacity_(other.capacity_) {
        std::copy(other.begin(), other.end(), items_.Get());
    }

    SimpleVector(SimpleVector&& other) {
        size_ = std::exchange(other.size_, 0);
        capacity_ = std::exchange(other.capacity_, 0);
        items_.swap(other.items_);
    }

    SimpleVector& operator=(const SimpleVector& rhs) {
        if (this != &rhs) {
            SimpleVector tmp(rhs);
            swap(tmp);
        }
        return *this;
    }

    void PushBack(const Type& item) {
        if (size_ + 1 < capacity_) {
            items_[size_] = item;
            ++size_;
        }
        else {
            size_t new_capacity = (capacity_ == 0) ? 1 : 2 * capacity_;
            Reserve(new_capacity);
            items_[size_] = std::move(item);
            ++size_;
        }
    }
    
    void PushBack(Type&& item) {
        if (size_ + 1 < capacity_) {
            items_[size_] = std::move(item);
            ++size_;
        }
        else {
            size_t new_capacity = (capacity_ == 0) ? 1 : 2 * capacity_;
            Reserve(new_capacity);
            items_[size_] = std::move(item);
            ++size_;
        }
    }
                
    Iterator Insert(ConstIterator pos, const Type& value) {
        assert(pos >= begin() && pos < end());
        const auto shift = pos - begin();
        if (size_ == capacity_) {
            size_t new_capacity = (capacity_ == 0) ? 1 : 2 * capacity_;
            Reserve(new_capacity);
        }
        MoveItemsBackward(shift);
        items_[shift] = value;
        ++size_;
        return (begin() + shift);
    }
    
    Iterator Insert(ConstIterator pos, Type&& value) {
        const auto shift = pos - begin();
        if (size_ == capacity_) {
            size_t new_capacity = (capacity_ == 0) ? 1 : 2 * capacity_;
            Reserve(new_capacity);
        }
        MoveItemsBackward(shift);
        items_[shift] = std::move(value);
        ++size_;
        return (begin() + shift);
    }
    
    void PopBack() noexcept {
        assert(size_ > 0);
        if (!IsEmpty()) {
            --size_;
        }
    }

    Iterator Erase(ConstIterator pos) {
        assert(pos >= begin() && pos < end());
        const auto shift = pos - begin();
        std::move(begin() + shift + 1, end(), begin() + shift);
        --size_;
        return (begin() + shift);
    }
        
    void swap(SimpleVector& other) noexcept {
        items_.swap(other.items_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

    void Reserve(size_t new_capacity) {
        if (new_capacity > capacity_) {
            ArrayPtr<Type> tmp(new_capacity);
            std::move(begin(), end(), tmp.Get());
            items_.swap(tmp);
            capacity_ = new_capacity;
        }
    } 

    Iterator begin() noexcept {
        return items_.Get();
    }

    Iterator end() noexcept {
        return (begin() + size_);
    }

    ConstIterator begin() const noexcept {
        return cbegin();
    }

    ConstIterator end() const noexcept {
        return cend();
    }

    ConstIterator cbegin() const noexcept {
        return ConstIterator(items_.Get());
    }

    ConstIterator cend() const noexcept {
        return ConstIterator(begin() + size_);
    }

    private:
    ArrayPtr<Type> items_;
    size_t size_ = 0;
    size_t capacity_ = 0;
        
    void MoveItemsBackward(size_t shift) {
        std::move_backward(begin() + shift, end(), end() + 1);
    }
};

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    if (lhs.GetSize() != rhs.GetSize()) {
        return false;
    }
    return (std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end()));
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs == rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs > rhs);
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !((lhs < rhs) || (lhs == rhs));
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs < rhs);
}