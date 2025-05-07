using System.Collections.Concurrent;

namespace CsConcurency.Queue;

/// <summary>
/// A thread-safe queue implemented from scratch using a singly linked list,
/// protected by a Mutex and an AutoResetEvent for producer-consumer synchronization.
/// </summary>
public class MutexQueue<T> : IDisposable, IQueue<T>
{
    /// <summary>
    /// Internal node class representing each element in the queue.
    /// </summary>
    private class Node(T value)
    {
        public readonly T Value = value;     // Value stored in the node
        public Node? Next;  // Pointer to the next node in the queue
    }

    // Head of the queue (front for dequeueing)
    private Node? _head;

    // Tail of the queue (end for enqueuing)
    private Node? _tail;

    // Count of elements in the queue
    private int _count;

    // Mutex used to guard access to the queue and ensure mutual exclusion
    private readonly Mutex _mutex = new();

    // Event used to signal availability of items to consumers
    private readonly AutoResetEvent _itemAvailable = new(false);

    // Track whether the object has been disposed
    private bool _disposed;

    /// <summary>
    /// Adds an item to the end of the queue in a thread-safe way.
    /// </summary>
    public void Enqueue(T item)
    {
        var node = new Node(item); // Wrap item in a node

        _mutex.WaitOne(); // Lock the queue for exclusive access
        try
        {
            if (_tail == null)
            {
                // Queue is empty: set head and tail to new node
                _head = _tail = node;
            }
            else
            {
                // Append to tail and move tail pointer
                _tail.Next = node;
                _tail = node;
            }

            _count++; // Increase item count

            _itemAvailable.Set(); // Signal that an item is now available
        }
        finally
        {
            _mutex.ReleaseMutex(); // Release the mutex regardless of exceptions
        }
    }

    
    /// <summary>
    /// Attempts to dequeue an item from the queue without blocking.
    /// </summary>
    /// <param name="obj"></param>
    /// <returns></returns>
    public bool TryDequeue(out T? obj)
    {
        _mutex.WaitOne(); // Lock the queue
        try
        {
            if (_head != null)
            {
                // There is an item available
                obj = _head.Value;

                // Move head to next node
                _head = _head.Next;

                // If queue is now empty, reset tail to null
                if (_head == null)
                    _tail = null;

                _count--; // Decrease count
                return true; // Successfully dequeued
            }
            else
            {
                obj = default; // No item available
                return false;
            }
        }
        finally
        {
            _mutex.ReleaseMutex(); // Always release lock
        }
    }

    /// <summary>
    /// Returns the number of items currently in the queue.
    /// Thread-safe but only reflects a snapshot.
    /// </summary>
    public int Count
    {
        get
        {
            _mutex.WaitOne(); // Lock to safely read _count
            try
            {
                return _count;
            }
            finally
            {
                _mutex.ReleaseMutex();
            }
        }
    }

    /// <summary>
    /// Checks if the queue is empty.
    /// </summary>
    public bool IsEmpty => Count == 0;



    /// <summary>
    /// Disposes of resources used by the queue.
    /// </summary>
    public void Dispose()
    {
        if (_disposed) return;
        _disposed = true;

        // Clean up system handles
        _mutex.Dispose();
        _itemAvailable.Dispose();

        GC.SuppressFinalize(this); // Suppress finalization
    }
}
