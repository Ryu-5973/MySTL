#ifndef MYSTL_LIST_H
#define MYSTL_LIST_H

// 模板类list
// list是一个双向链表

// 异常保证
// mystl::list<T>满足基本异常保证,部分函数无异常保证,并对以下等函数做强异常安全保证
// * emplace_front
// * emplace
// * push_front
// * push_back
// * insert

#include <initializer_list>

#include "iterator.h"
#include "memory.h"
#include "functional.h"
#include "util.h"
#include "exceptdef.h"

namespace mystl
{

// list_node 共有继承list_node_bace
// list_node_base负责链表中双向指针部分
// list_node在base的基础上扩充了数据部分

template<typename T> struct list_node_base;
template<typename T> struct list_node;

template <typename T>
struct node_traits{
    typedef list_node_base<T>   *base_ptr;
    typedef list_node<T>        *node_ptr;
};

// list 节点结构

// list中的node(双向链表)部分
template <typename T>
struct list_node_base {
    // list_node_base<T>*
    typedef typename node_traits<T>::base_ptr base_ptr;
    // list_node<T>*
    typedef typename node_traits<T>::node_ptr 

    base_ptr prev;  // 前向指针
    base_ptr next;  // 后向指针

    // list_node_base向list_node的转换
    node_ptr as_node() {
        return static_cast<node_ptr>(self());
    }

    void unlink() {
        prev = next = self();
    }

    // 返回list_node_base*
    base_ptr self() {
        return this;
    }
};

template <typename T>
struct list_node : public list_node_base<T> {
    typedef typename node_traits<T>::base_ptr base_ptr;
    typedef typename node_traits<T>::node_ptr node_ptr;

    T value; // list 的数据域
    // 双向链表指针域继承自list_node_base

    list_node() = default;
    list_node(const T& v): value(v) {}
    list_node(T&& v): value(mystl::move(v)) {}

    // 返回list_node_base*
    base_ptr as_base() {
        return static_cast<base_ptr>(this);
    }
    // 返回自身指针list_node*
    node_ptr self() {
        return this;
    }
};

// list 迭代器设计
template <typename T>
struct list_iterator : public mystl::iterator<bidirectional_iterator_tag, T> {
    // 双向链表要用bidirectional_iterator_tag
    typedef T                                   value_type;
    typedef T*                                  pointer;
    typedef T&                                  reference;
    typedef typename node_traits<T>::base_ptr   base_ptr;
    typedef typename node_traits<T>::node_ptr   node_ptr;
    typedef list_iterator<T>                    self;

    base_ptr node_;     // 指向当前节点

    // 构造函数
    list_iterator() = default;
    list_iterator(base_ptr x): node_(x) {}
    list_iterator(node_ptr x): node_(x->as_base()) {}
    list_iterator(const list_iterator& rhs): node_(rhs.node_) {}

    // 重载操作符
    reference operator*() const {
        MYSTL_DEBUG(node_ != nullptr);
        return node_->as_node()->value;
    }
    pointer operator->() const {
        return &(operator*());
    }

    self& operator++() {    // 前置递增 
        MYSTL_DEBUG(node_ != nullptr);
        node_ = node_->next;
        return *this;
    }

    self operator++(int) {  // 后置递增
        self tmp = *this;
        ++*this;
        return tmp;
    }

    self& operator--() { // 前置递减
        MYSTL_DEBUG(node_ != nullptr);
        node_ = node_->prev;
        return *this;
    }

    self operator--(int) {  // 后置递减
        self tmp = *this;
        --*this;
        return tmp;
    }

    // 重载比较运算符
    bool operator==(const self& rhs) const {
        return node_ == rhs.node_;
    }
    bool operator!=(const self& rhs) const {
        return node_ != rhs.node_;
    }
};

// 常量迭代器
template <typename T>
struct list_const_iterator : public iterator<bidirectional_iterator_tag, T> {
    typedef T                                   value_type;
    typedef T*                                  pointer;
    typedef T&                                  reference;
    typedef typename node_traits<T>::base_ptr   base_ptr;
    typedef typename node_traots<T>::node_ptr   node_ptr;
    typedef list_const_iterator<T>                    self;

    base_ptr node_;     // 指向当前节点    

    // 构造函数
    list_const_iterator() = default;
    list_const_iterator(base_ptr x): node_(x) {}
    list_const_iterator(node_ptr x): node_(x->as_base()) {}
    list_const_iterator(const list_iterator& rhs): node_(rhs.node_) {};
    list_const_iterator(const list_const_iterator& rhs): node_(rhs.node_) {}

    // 重载操作符
    reference operator*() const {
        MYSTL_DEBUG(node_ != nullptr);
        return node_->as_node()->value;
    }
    pointer operator->() const {
        return &(operator*());
    }

    self& operator++() {    // 前置递增 
        MYSTL_DEBUG(node_ != nullptr);
        node_ = node_->next;
        return *this;
    }

    self operator++(int) {  // 后置递增
        self tmp = *this;
        ++*this;
        return tmp;
    }

    self& operator--() { // 前置递减
        MYSTL_DEBUG(node_ != nullptr);
        node_ = node_->prev;
        return *this;
    }

    self operator--(int) {  // 后置递减
        self tmp = *this;
        --*this;
        return tmp;
    }

    // 重载比较运算符
    bool operator==(const self& rhs) const {
        return node_ == rhs.node_;
    }
    bool operator!=(const self& rhs) const {
        return node_ != rhs.node_;
    }
};

// 模板类list
template <typename T>
class list {
public:
    // 构造器
    typedef mystl::allocator<T>                     allocator_type;
    typedef mystl::allocator<T>                     data_allocator;
    typedef mystl::allocator<list_node_base<T>>     base_allocator;
    typedef mystl::allocator<list_node<T>>          node_allocator;

    // 类型(用于type traits)
    // typedef typename allocator_type::value_type      value_type;
    // typedef typename allocator_type::pointer         pointer;
    // typedef typename allocator_type::const_pointer   const_pointer;
    // typedef typename allocator_type::reference       reference;
    // typedef typename allocator_type::const_reference const_reference;
    // typedef typename allocator_type::size_type       size_type;
    // typedef typename allocator_type::difference_type difference_type;
    typedef T            value_type;
	typedef T*           pointer;
	typedef const T*     const_pointer;
	typedef T&           reference;
	typedef const T&     const_reference;
	typedef size_t       size_type;
	typedef ptrdiff_t    difference_type;

    // 迭代器
    typedef list_iterator<T>                            iterator;
    typedef list_const_iterator<T>                      const_iterator;
    typedef mystl::reverse_iterator<iterator>           reverse_iterator;
    typedef mystl::reverse_iterator<const_iterator>     const_reverse_iterator;

    typedef typename node_traits<T>::base_ptr           base_ptr;
    typedef typename node_traits<T>::node_ptr           node_ptr;

    allocator_type get_allocator() {
        return node_allocator();
    } 

private:
    base_ptr    node_;  // 指向末尾结点
    size_type   size_;  // 大小

public:
    // 构造,复制,移动,析构函数
    list() {
        fill_init(0, value_type());
    }
    explicit list(size_type n) {
        fill_init(n, value_type());
    }
    list(size_type n, const T& value) {
        fill_init(n, value);
    }

    template <typename Iter, typename std::enable_if<
        mystl::is_input_iterator<Iter>::value, int>::type = 0>
    list(Iter first, Iter last) {
        copy_init(first, last);
    }

    list(std::initializer_list<T> ilist) {
        copy_init(ilsit.begin(), ilist.end());
    }

    list(const list& rhs) {
        copy_init(rhs.cbegin(), rhs.cend());
    }

    list(list&& rhs) noexcept
        : node_(rhs.node_), size(rhs.size) {
        rhs.node_ = nullptr;
        rhs.size_ = 0;
    }

    list& operator=(const list& rhs) {
        // 自赋值判断
        if(this != &rhs) {
            assign(rhs.begin(), rhs.end());
        }
        return *this;
    }

    list& operator=(list&& rhs) noexcept {
        clear();
        splice(end(), rhs);
        return *this;
    }

    list& operator=(std::initializer_list<T> ilist) {
        list tmp(ilist.begin(), ilist.end());
        swap(tmp);
        return *this;
    }

    ~list() {
        if(node_) {
            clear();
            base_allocator::deallocate(node_);
            node_ = nullptr;
            size_ = 0;
        }
    }

public:
    // 迭代器相关操作
    iterator begin() noexcept {
        return node_->next;
    }
    const_iterator begin() const noexcept {
        return node_->next;
    }
    iterator end() noexcept {
        return node_;
    }
    const_iterator end() const noexcept {
        return node_;
    }

    reverse_iterator rbegin() noexcept {
        return reverse_iterator(end());
    }
    const_reverse_iterator rbegin() const noexcept {
        return reverse_iterator(end());
    }
    reverse_iterator rend() noexcept {
        return reverse_iterator(begin());
    }
    const_reverse_iterator rend() const noexcept {
        return reverse_iterator(begin());
    }

    const_iterator cbegin() const noexcept {
        return begin();
    }
    const_iterator cend() const noexcept {
        return end();
    }
    const_reverse_iterator crbegin() const noexcept {
        return rbegin();
    }
    const_reverse_iterator crend() const noexcept {
        return rend();
    }

    // 容量相关操作
    bool empty() const noexcept {
        return node_->next == node_; 
    }

    size_type size() const noexcept {
        return size_;
    } 

    size_type max_size() const noexcept {
        return static_cast<size_type>(-1);
    }

    // 访问元素相关操作
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
        return *(--end());
    }

    const_reference back() const {
        MYSTL_DEBUG(!empty());
        return *(--end());
    }

    // 调整容器相关的操作
    // assign
    void assign(size_type n, const value_type& value) {
        fill_assign(n, value);
    }

    template <typename Iter, typename std::enable_if<
        mystl::is_input_iterator<Iter>::value, int>::type = 0>
    void assign(Iter first, Iter last) {
        copy_assign(first, last);
    }

    void assign(std::initializer_list<T> ilist) {
        copy_assign(ilist.begin(), ilist.end());
    }

    template <typename ...Args>
    void emplace_front(Args&& ...args) {
        THROW_LENGTH_ERROR_IF(size_ > max_size() - 1, "list<T>'s size too big");
        auto link_node = create_node(mystl::forward<Args>(args)...);
        link_nodes_at_front(link_node->as_base(), link_node->as_base());
        ++ size;
    }

    template <typename ...Args>
    void emplace_back(Args&& ...args) {
        THROW_LENGTH_ERROR_IF(size_ > max_size() - 1, "list<Y>'s size too big");
        auto link_node == create_node(mystl::forward<Args>(args)...);
        link_nodes_at_back(link_node->as_base(), link_node->as_base());
        ++ size_;
    }
    
    template <typename ...Args>
    iterator emplace(const_iterator pos, Args&& ...args) {
        THROW_LENGTH_ERROR_IF(size_ > max_size() - 1, "list<T>'s size too big");
        auto link_node = create_node(mystl::forward<Args>(args)...);
        link_nodes(pos.node_, link_node->as_base(), link_node->as_base());
        ++ size_;
        return iterator(link_node);
    }

    // insert

    iterator insert(const_iterator pos, const value_type& value) {
        THROW_LENGTH_ERROR_IF(size_ > max_size() - 1, "list<T>'s size too big");
        auto link_node = create_node(mystl::forward<Args>(args)...);
        ++ size_;
        return link_iter_node(pos, link_node->as_base());
    }

    interator insert(const_iterator pos, velue_type&& value) {
        THROW_LENGTH_ERROR_IF(size_ > max_size() - 1, "list<T>'s size too big");
        auto link_node = create_node(mystl::forward<Args>(args)...);
        ++ size_;
        return link_iter_node(pos, link_node->as_base());
    }

    iterator insert(const_iterator pos, size_type n, const value_type& value) { 
        THROW_LENGTH_ERROR_IF(size_ > max_size() - n, "list<T>'s size too big");
        return fill_insert(pos, n, value); 
    }

    template <typename Iter, typename std::enable_if<
        mystl::is_input_iterator<Iter>::value, int>::type = 0>
    iterator insert(const_iterator pos, Iter first, Iter last) { 
        size_type n = mystl::distance(first, last);
        THROW_LENGTH_ERROR_IF(size_ > max_size() - n, "list<T>'s size too big");
        return copy_insert(pos, n, first); 
    }

    // push_front / push_back
    void push_front(const value_type& value){
        THROW_LENGTH_ERROR_IF(size_ > max_size() - 1, "list<T>'s size too big");
        auto link_node = create_node(value);
        link_nodes_at_front(link_node->as_base(), link_node->as_base());
        ++ size_;
    }

    void push_front(value_type&& value){
        emplace_front(mystl::move(value));
    }

    void push_back(const value_type& value){
        THROW_LENGTH_ERROR_IF(size_ > max_size() - 1, "list<T>'s size too big");
        auto link_node = create_node(value);
        link_nodes_at_back(link_node->as_base(), link_node->as_base());
        ++ size_;
    }

    void push_back(value_type&& value){
        emplace_back(mystl::move(value));
    }

    // pop_front / pop_back
    void pop_front() {
        MYSTL_DEBUG(!empty());
        auto n = node_->next;
        unlink_nodes(n, n);
        destroy_node(n->as_node());
        -- size_;
    }

    void pop_back() {
        MYSTL_DEBUG(!empty());
        auto n = node_->prev;
        unlink_nodes(n, n);
        destroy_node(n->as_node());
        -- size_;
    }

    // erase / clear
    iterator erase(const_iterator pos);
    iterator erase(const_iterator first, const_iterator last);

    void clear();

    // resize

    void resize(size_type new_size) {
        resize(new_size, value_type());
    }

    void resize(size_type, const value_type& value);

    void swap(list& rhs) noexcept {
        mystl::swap(node_, rhs.node_);
        mystl::swap(size_, rhs.size_);
    }

    // list 相关操作
    void splice(const_iterator pos, list& other);
    void splice(const_iterator pos, list& other, const_iterator it);
    void splice(const_iterator pos, list& other, const_iterator first, const_iterator last);

    void remove(const value_type& value) {
        remove_if([&](const value_type&v){
            return v == value;
        });
    }

    template <typename UnaryPredicate>
    void remove_id(UnaryPredicate pred);

    void unique(){
        unique(mystl::equal_to<T>());
    }

    template <typename BinaryPredicate>
    void unique(BinaryPredicate pred);

    void merge(list& x) {
        merge(x, mystl::less<T>());
    }

    template <typename Compare>
    void merge(list& x, Compare comp);

    void sort(){
        list_sort(begin(), end(), size(), mystl::less<T>());
    }
    template <typename Compared>
    void sort(Compared comp) {
        list_sort(begin(), end(), size(), comp);
    }

    void reverse();

private:
    // helper functions

    // create / destroy node
    template <typename ...Args>
    node_ptr create_node(Arg&& ...args);
    void destroy_node(node_ptr p);

    // initialize
    void fill_init(size_t n, const value_type& value);
    template <typename Iter>
    void copy_init(Iter first, Iter last);

    // link / unlink    
    iterator  link_iter_node(const_iterator pos, base_ptr node);
    void      link_nodes(base_ptr p, base_ptr first, base_ptr last);
    void      link_nodes_at_front(base_ptr first, base_ptr last);
    void      link_nodes_at_back(base_ptr first, base_ptr last);
    void      unlink_nodes(base_ptr f, base_ptr l);

    // assign
    void      fill_assign(size_type n, const value_type& value);
    template <typename Iter>
    void      copy_assign(Iter first, Iter last);

    // insert
    iterator  fill_insert(const_iterator pos, size_type n, const value_type& value);
    template <typename Iter>
    iterator  copy_insert(const_iterator pos, size_type n, Iter first);

    // sort
    template <typename Compared>
    iterator  list_sort(iterator first, iterator last, size_type n, Compared comp);
}; 


}

#endif