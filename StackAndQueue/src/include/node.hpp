#pragma once

#include <memory>

template <class T>
struct Node {
    T data;
    std::unique_ptr<Node> next;

    Node(T value) : data(std::move(value)), next(nullptr) {}
};