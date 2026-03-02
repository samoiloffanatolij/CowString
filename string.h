#include <algorithm>
#include <cstring>
#include <iostream>

class CowString {
public:
    CowString();

    CowString(size_t n, char c);

    CowString(const char* c_str);

    CowString(const CowString& cow_str);

    ~CowString();

    [[nodiscard]] size_t length() const {
        return length_;
    }

    [[nodiscard]] size_t size() const {
        return length_;
    }

    [[nodiscard]] size_t capacity() const {
        return capacity_;
    }

    [[nodiscard]] bool empty() const {
        return length_ == 0;
    }

    char* data() {
        copy();
        return data_;
    }

    [[nodiscard]] const char* data() const {
        return data_;
    }

    char& front() {
        copy();
        return data_[0];
    }

    [[nodiscard]] const char& front() const {
        return data_[0];
    }

    char& back() {
        copy();
        return data_[length_ - 1];
    }

    [[nodiscard]] const char& back() const {
        return data_[length_ - 1];
    }

    void clear() {
        if (ref_count() > 1) {
            --ref_count();
        }
        length_ = 0;
        data_[0] = '\0';
    }

    void shrink_to_fit() {
        if (capacity_ > length_) {
            realloc(length_);
        }
    }

    void push_back(char c);

    void pop_back() {
        copy();
        data_[--length_] = '\0';
    }

    [[nodiscard]] size_t find(const CowString& sub) const;

    [[nodiscard]] size_t rfind(const CowString& sub) const;

    [[nodiscard]] CowString substr(size_t start, size_t count) const;

    char& operator[](size_t ind) {
        copy();
        return data_[ind];
    }

    const char& operator[](size_t ind) const {
        return data_[ind];
    }

    CowString& operator=(const CowString& cow_str);

    CowString& operator+=(const CowString& cow_str);

    CowString& operator+=(char c) {
        push_back(c);
        return *this;
    }

    CowString operator+(const CowString& cow_str) const {
        return CowString(*this) += cow_str;
    }

    CowString operator+(char c) const {
        return CowString(*this) += c;
    }

    bool operator==(const CowString& cow_str) const {
        return length_ == cow_str.length_ && std::memcmp(data_, cow_str.data_, length_) == 0;
    }

    bool operator<(const CowString& cow_str) const {
        int cmp = std::memcmp(data_, cow_str.data_, std::min(length_, cow_str.length_));
        return cmp < 0 || (cmp == 0 && length_ < cow_str.length_);
    }

    bool operator>(const CowString& cow_str) const {
        return cow_str < *this;
    }

    bool operator<=(const CowString& cow_str) const {
        return !(*this > cow_str);
    }

    bool operator>=(const CowString& cow_str) const {
        return !(*this < cow_str);
    }

private:
    void copy();

    size_t& ref_count();

    void realloc(size_t new_capacity);

    void swap(CowString& cow_str);

    static constexpr size_t TYPE_SIZE = 8;

    size_t length_;
    size_t capacity_;
    char* data_;
};

CowString::CowString() : length_(0), capacity_(0), data_(new char[1 + TYPE_SIZE]) {
    data_[0] = '\0';
    ref_count() = 1;
}

CowString::CowString(size_t n, char c) : length_(n), capacity_(n),
                                         data_(new char[n + 1 + TYPE_SIZE]) {
    ref_count() = 1;
    std::fill(data_, data_ + n, c);
    data_[n] = '\0';
}

CowString::CowString(const char* c_str)
        : length_(std::strlen(c_str)), capacity_(std::strlen(c_str)),
          data_(new char[capacity_ + 1 + TYPE_SIZE]) {
    ref_count() = 1;
    std::copy(c_str, c_str + length_ + 1, data_);
}

CowString::CowString(const CowString& cow_str)
        : length_(cow_str.length_),
          capacity_(cow_str.capacity_),
          data_(cow_str.data_) {
    ++ref_count();
}

CowString::~CowString() {
    if (--ref_count() == 0) {
        delete[] data_;
    }
}

void CowString::push_back(char c) {
    if (length_ >= capacity_) {
        realloc(capacity_ == 0 ? 1 : capacity_ * 2);
    } else {
        copy();
    }
    data_[length_++] = c;
    data_[length_] = '\0';
}

size_t CowString::find(const CowString& sub) const {
    for (const char* ptr = data_; ptr <= data_ + length_ - sub.length_; ++ptr) {
        if (std::memcmp(ptr, sub.data_, sub.length_) == 0) {
            return ptr - data_;
        }
    }
    return length_;
}

size_t CowString::rfind(const CowString& sub) const {
    for (const char* ptr = data_ + length_ - sub.length_; ptr >= data_; --ptr) {
        if (std::memcmp(ptr, sub.data_, sub.length_) == 0) {
            return ptr - data_;
        }
    }
    return length_;
}

CowString CowString::substr(size_t start, size_t count) const {
    if (count == -1ULL) {
        return substr(start, length_ - start);
    }
    if (start + count > length_) {
        count = length_ - start;
    }
    CowString sub(count, ' ');
    std::copy(data_ + start, data_ + start + count, sub.data_);
    return sub;
}

CowString& CowString::operator=(const CowString& cow_str) {
    if (*this != cow_str) {
        CowString temp = cow_str;
        swap(temp);
    }
    return *this;
}

CowString& CowString::operator+=(const CowString& cow_str) {
    if (cow_str.length_ == 0) {
        return *this;
    }
    if (length_ + cow_str.length_ > capacity_) {
        realloc(std::max(length_ * 2, length_ + cow_str.length_));
    } else {
        copy();
    }
    std::copy(cow_str.data_, cow_str.data_ + cow_str.length_,
              data_ + length_);
    length_ += cow_str.length_;
    data_[length_] = '\0';
    return *this;
}

void CowString::copy() {
    if (ref_count() > 1) {
        realloc(capacity_);
    }
}

size_t& CowString::ref_count() {
    return *reinterpret_cast<size_t*>(data_ + capacity_ + 1);
}

void CowString::realloc(size_t new_capacity) {
    char* new_data = new char[new_capacity + 1 + TYPE_SIZE];
    std::copy(data_, data_ + length_ + 1, new_data);
    if (ref_count() > 1) {
        --ref_count();
    } else {
        delete[] data_;
    }
    data_ = new_data;
    capacity_ = new_capacity;
    ref_count() = 1;
}

void CowString::swap(CowString& cow_str) {
    std::swap(data_, cow_str.data_);
    std::swap(length_, cow_str.length_);
    std::swap(capacity_, cow_str.capacity_);
}

std::istream& operator>>(std::istream& in, CowString& cow_str) {
    cow_str.clear();

    for (char c; in.get(c);) {
        if (isspace(c) == 0 && c != '\n' && c != EOF) {
            cow_str.push_back(c);
            break;
        }
    }

    for (char c; in.get(c) && isspace(c) == 0 && c != '\n' && c != EOF; cow_str.push_back(c)) {
    }

    return in;
}

std::ostream& operator<<(std::ostream& out, const CowString& cow_str) {
    return out.write(cow_str.data(), cow_str.size());
}

CowString operator+(char c, const CowString& cow_str) {
    CowString temp(1, c);
    temp += cow_str;
    return temp;
}

CowString operator+(const char* c_str, const CowString& cow_str) {
    return CowString(c_str) + cow_str;
}

bool operator==(const char* c_str, const CowString& cow_str) {
    return CowString(c_str) == cow_str;
}

bool operator<(const char* c_str, const CowString& cow_str) {
    return CowString(c_str) < cow_str;
}

bool operator>(const char* c_str, const CowString& cow_str) {
    return CowString(c_str) > cow_str;
}

bool operator<=(const char* c_str, const CowString& cow_str) {
    return CowString(c_str) <= cow_str;
}

bool operator>=(const char* c_str, const CowString& cow_str) {
    return CowString(c_str) >= cow_str;
}
