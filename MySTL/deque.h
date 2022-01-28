#ifndef MYSTL_DEQUE_H
#define MYSTL_DEQUE_H

// 模板类deque

// 异常保证:
// mystl::deque<T> 满足基本异常保证，部分函数无异常保证，并对以下等函数做强异常安全保证：
//   * emplace_front
//   * emplace_back
//   * emplace
//   * push_front
//   * push_back
//   * insert

#include <initializer_list>

#include "iterator.h"
#include "memory.h"
#include "util.h"
#include "exceptdef.h"

namespace mystl{

#ifdef max
#pragma message("#undefing marco max")
#undef max
#endif // max

#ifdef min
#pragma message("#undefing marco min")
#undef min
#endif // min

// deque map 初始化的大小
#ifndef DEQUE_MAP_INIT_SIZE
#define DEQUE_MAP_INIT_SIZE 8
#endif
// #e DEQUE_MAP_INIT_SIZE

// 缓冲区大小
template <typename T>
struct deque_buf_size{
    static constexpr size_t value = sizeof(T) < 256 ? 4096 / sizeof(T): 16;
};

// deque迭代器设计
template <typename T, typename Ref, typename Ptr>
struct deque_iterator : public iterator<random_access_iterator_tag, T>{
    typedef deque_iterator<T, T&, T*>               iterator;
    typedef deque_iterator<T, const T&, const T*>   const_iterator;
    typedef deque_iterator                          self;

    typedef T           value_type;
    typedef Ptr         pointer;
    typedef Ref         reference;
    typedef size_t      size_type;
    typedef ptrdiff_t   difference_type;
    typedef T*          value_pointer;
    typedef T**         map_pointer;

    static const size_type buffer_size = deque_buf_size<T>::value;

    // 迭代器所含成员数据(16byte)
    value_pointer   cur;        // 指向所在缓冲区的当前元素
    value_pointer   first;      // 指向所在缓冲区的头部
    value_pointer   last;       // 指向所在缓冲区的尾部
    map_pointer     node;       // 缓冲区所在节点

    // 构造,复制,移动函数
    deque_iterator() noexcept 
        :cur(nullptr), first(nullptr), last(nullptr), node(nullptr) {}

    deque_iterator(value_pointer v, map_pointer n)
        :cur(v), first(*n), last(*n + buffer_size), node(n) {}

    deque_iterator(const iterator& rhs)
        :cur(rhs.cur), first(rhs.first), last(rhs.last), node(rhs.node) {}
    
    deque_iterator(iterator&& rhs) noexcept
        :cur(rhs.cur), first(rhs.first), last(rhs.last), node(rhs.last) {
        rhs.cur = nullptr;
        rhs.first = nullptr;
        rhs.last = nullptr;
        rhs.node = nullptr;
    }

    deque_iterator(const const_iterator& rhs)
        :cur(rhs.cur), first(rhs.first), last(rhs.last), node(rhs.node) {}

    self& operator=(const iterator& rhs) {
        if(this != &rhs) {
            cur = rhs.cur;
            first = rhs.first;
            last = rhs.last;
            node = rhs.node;
        }
        return *this;
    }

    // 转到另一个缓冲区
    void set_node(map_pointer new_node) {
        node = new_node;
        first = *new_node;
        last = *new_node + buffer_size;
    }

    // 重载运算符
    reference operator*() const {
        return *cur;
    }
    pointer operator->() const {
        return cur;
    }

    difference_type operator-(const self& x) const {
        return static_cast<difference_type>(buffer_size) * (node - x.node)
            + (cur - first) - (x.cur - x.first);
    }

    self& operator++() {
        ++ cur;
        if(cur == last) {   // 如果达到缓冲区的尾
            set_node(node + 1);
            cur = first;
        }
        return *this;
    }

    self operator++(int) {
        self tmp = *this;
        ++ *this;
        return tmp;
    }

    self operator--() {
        if(cur == first) {
            set_node(node - 1);
            cur = last;
        }
        -- cur;
        return *this;
    }

    self operator--(int) {
        self tmp = *this;
        -- *this;
        return tmp;
    }

    self& operator+=(difference_type n) {
        const auto offset = n + (cur - first);
        if(offset >= 0 && offset < static_cast<difference_type>(buffer_size)) { // 仍在当前缓冲区
            cur += n;
        }else { // 跳到其他的缓冲区
            const auto node_offset = offset > 0
                ? offset / static_cast<difference_type>(buffer_size)    // 往后走
                : -static_cast<difference_type>((-offset - 1) / buffer_size) - 1;   // 往前走
            set_node(node + node_offset);
            cur = first + (offset - node_offset * static_cast<difference_type>(buffer_size));
        }
        return *this;
    }
    self operator+(difference_type n) const {
        self tmp = *this;
        return tmp += n;
    }
    self& operator-=(difference_type n) {
        return *this += -n;
    }
    self operator-(difference_type n) const{
        self tmp = *this;
        return tmp - n;
    }

    reference operator[](difference_type n) const {
        return *(*this + n);
    }

    // 重载比较运算符
    bool operator==(const self& rhs) const {
        return cur == rhs.cur;
    }
    bool operator<(const self& rhs) const {
        return node == rhs.node ? cur < rhs.cur : node < rhs.node;
    }
    bool operator!=(const self& rhs) const {
        return !(*this == rhs);
    }
    bool operator> (const self& rhs) const { 
        return rhs < *this;
    }
    bool operator<=(const self& rhs) const { 
        return !(rhs < *this); 
    }
    bool operator>=(const self& rhs) const { 
        return !(*this < rhs); 
    }
};

// 模板类deque
// 模板参数代表数据类型
template <typename T>
class deque{
public:
    // deque的型别定义
    typedef mystl::allocator<T>                 allocator_type;
    typedef mystl::allocator<T>                 data_allocator;
    typedef mystl::allocator<T*>                map_allocator;

    typedef typename allocator_type::value_type      value_type;
    typedef typename allocator_type::pointer         pointer;
    typedef typename allocator_type::const_pointer   const_pointer;
    typedef typename allocator_type::reference       reference;
    typedef typename allocator_type::const_reference const_reference;
    typedef typename allocator_type::size_type       size_type;
    typedef typename allocator_type::difference_type difference_type;
    typedef pointer*                                 map_pointer;
    typedef const_pointer*                           const_map_pointer;

    typedef typename deque_iterator<T, T&, T*>              iterator;
    typedef typename deque_iterator<T, const T&, const T*>  const_iterator;
    typedef mystl::reverse_iterator<iterator>               reverse_iterator;
    typedef mystl::reverse_iterator<const_iterator>         const_reverse_iterator;

    allocator_type get_allocator() {
        return allocator_type();
    }

    static const size_type buffer_size = deque_buf_size<T>::value;

private:
    // deque的4个数据成员(40byte)
    iterator        begin_;     // 指向第一个节点
    iterator        end_;       // 指向最后一个节点
    map_pointer     map_;       // 指向一块map,map是一个指针数组,每个指针指向一个buffer
    size_type       map_size_;  // map内指针的数目

public:
    // 构造,复制.移动.析构函数

    deque(){
        fill_init(0, value_type());
    }

    explicit deque(size_type n) {
        fill_init(n, value_type());
    }

    deque(size_type n, const value_type& value) {
        fill_init(n, value);
    }

    template <typename Iter, typename std::enable_if<
        mystl::is_input_iterator<Iter>::value, int>::type = 0>
    deque(Iter first, Iter last) {
        copy_init(first, last, iterator_category(first));
    }

    deque(std::initializer_list<value_type> ilist) {
        copy_init(ilist.begin(), ilist.end(), mystl::forward_iterator_tag());
    }

    deque(const deque& rhs) {
        copy_init(rhs.begin(), rhs.end(), mystl::forward_iterator_tag());
    }

    deque(deque&& rhs) noexcept
        :begin_(mystl::move(rhs.begin_)), 
        end_(mystl::move(rhs.end_)),
        map_(rhs.map_),
        map_size_(rhs.map_size_) {
        rhs.map_ = nullptr;
        rhs.map_size_ = 0;
    }

    deque& operator=(const deque& rhs);
    deque& operator=(deque&& rhs);

    deque& operator=(std::initializer_list<value_type> ilist) {
        deque tmp(ilist);
        swap(tmp);
        return *this;
    }

    ~deque() {
        if(map_ != nullptr) {
            clear();
            data_allocator::deallocate(*begin_.node, buffer_size);
            *begin_.node = nullptr;
            map_allocator::deallocate(map_, map_size_);
            map_ = nullptr;
        }
    }
public:
    // 迭代器相关操作
    iterator begin() noexcept{
        return begin_;
    }

    const_iterator begin() const noexcept{
        return begin_;
    }

    iterator end() noexcept{
        return end_;
    }

    const_iterator end() const noexcept{
        return end_;
    }

    reverse_iterator rbegin() noexcept{
        return reverse_iterator(end());
    }

    const_reverse_iterator rbegin() const noexcept{
        return reverse_iterator(end());
    }

    reverse_iterator rend() noexcept{
        return reverse_iterator(begin());
    }

    const_reverse_iterator rend() const noexcept{
        return reverse_iterator(begin());
    }

    const_iterator cbegin() const noexcept{
        return begin();
    }

    const_iterator cend() const noexcept{
        return end();
    }
    
    const_reverse_iterator crbegin() const noexcept{
        return rbegin();
    }

    const_reverse_iterator crend() const noexcept{
        return rend();
    }

    // 访问元素的相关操作
    reference operator[](size_type n) {
        MYSTL_DEBUG(n < size());
        return begin_[n];
    }

    const_reference operator[](size_type n) const {
        MYSTL_DEBUG(n < size());
        return begin_[n];
    }

    reference at (size_type n) {
        THROW_LENGTH_ERROR_IF(!(n < size()), "deque<T>::at() subscript out of range");
        return (*this)[n];
    }

    const_reference at (size_type n) const {
        THROW_LENGTH_ERROR_IF(!(n < size()), "deque<T>::at() subscript out of range");
        return (*this)[n];
    }

    reference front() {
        MYSTL_DEBUG(!empty());
        return *begin();
    }

    const_reference front() const {
        MYSTL_DEBUG(!empty());
        return *begin();
    }

    reference back() {
        MYSTL_DEBUG(!empty());
        return *(end() - 1);
    }

    const_reference back() const {
        MYSTL_DEBUG(!empty());
        return *(end() - 1);
    }

    // 修改容器相关操作

    // assign

    void assign(size_type n, const value_type& value) {
        fill_assign(n, value);
    }

    template <typename Iter, typename std::enable_if<
        mystl::is_input_iterator<Iter>::value, int>::type = 0>
    void assign(Iter first, Iter last) {
        copy_assign(first, last, iterator_category(first));
    }

    void assign(std::initializer_list<value_type> ilist) {
        copy_assign(ilist.begin(), ilist.end(), mystl::forward_iterator_tag{});
    }

    // emplace_front / emplace_back /emplace

    template <typename ...Args>
    void emplace_front(Args&& ...args);
    template <typename ...Args>
    void emplace_back(Args&& ...args);
    template <typename ...Args>
    void emplace(Args&& args);

    // push_front / push_back

    void push_front(const value_type& value);
    void push_back(const value_type& value);

    void push_front()(value_type&& value) {
        emplace_front(mystl::move(value));
    }
    void push_back(value_type&& value) {
        emplace_back(mystl::move(value));
    }

    // pop_back / pop_front

    void pop_front();
    void pop_back();

    //insert
    iterator insert(iterator position, const value_type& value);
    iterator insert(iterator position, value_type&& value);
    void insert(iterator position, size_type n, const value_type& value);
    template <typename Iter, typename std::enable_if<
        mystl::is_input_iterator<Iter>::value, int>::type = 0>
    void insert(iterator position, Iter first, Iter last) {
        insert_dispatch(position, first, last, iterator_category(first));
    }

    // erase / clear

    iterator erase(iterator position);
    iterator erase(first, iterator last);
    void clear();

    // swap

    void swap(deque& rhs) noexcept;

private:
    // helper function

    // create node / destroy node
    map_pointer create_map(size_type size);
    void create_buffer(map_pointer nstart, map_pointer nfinish);
    void destroy_buffer(map_pointer nstart, map_pointer nfinish);

    // initialize
    void map_init(size_type nelem);
    void fill_init(size_type n, const value_type& value);
    template <typename Iter>
    void copy_init(Iter first, Iter last, input_iterator_tag);
    template <typename Iter>
    void copy_init(Iter first, Iter last, forward_iterator_tag);

    // assign
    void fill_assign(size_type n, const value_type& value);
    template <typename Iter>
    void copy_assign(Iter first, Iter end, input_iterator_tag);
    template <typename Iter>
    void copy_assign(Iter first, Iter last, forward_iterator_tag);

    // insert
    template <typename ...Args>
    iterator insert_aux(iterator position, Args&& args);
    void fill_insert(iterator position, size_type n, const value_type& value);
    template <typename Iter>
    void copy_insert(iterator position, Iter first, Iter last, size_type n);
    template <typename Iter>
    void insert_dispatch(iterator position, Iter first, Iter last, input_iterator_tag);
    template <typename Iter>
    void insert_dispatch(iterator position, Iter first, Iter last, forward_iterator_tag);

    // reallocate
    void require_capacity(size_type n, bool front);
    void reallocate_map_at_front(size_type need);
    void reallocate_map_at_back(size_type need);
};

// 移动赋值运算符
template <typename T>
deque<T>& deque<T>::operator=(deque&& rhs) {
    clear();
    begin_ = mystl::move(rhs.begin_);
    end_ = mystl::move(rhs.end_);
    map_ = rhs.map_;
    map_size_ = rhs.map_size_;
    rhs.map_ = nullptr;
    rhs.map_size_ = 0;
    return *this;
}


}   // namespace mystl

#endif  // MYSTL_DEQUE_H