#pragma once

#include <memory>

template <class T>
struct node {
    T data;
    std::unique_ptr<node> next;

    explicit node(T value) : data(std::move(value)), next(nullptr) {}
};