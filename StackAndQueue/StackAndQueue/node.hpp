#pragma once

#include <memory>

template<class T> // maybe typename?
struct Node {
    T data;
    std::unique_ptr<Node> next;

    Node(T value) : data(std::move(value)), next(nullptr) {}
};