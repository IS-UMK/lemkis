namespace CsConcurency.Queue;

public class StConcurrentQueue<T> : IQueue<T>
{
    private class Node(T value)
    {
        public readonly T Value = value;
        public Node? Next;
    }

    private volatile Node _head;
    private volatile Node _tail;
    private volatile int _count;

    public StConcurrentQueue()
    {
        var dummy = new Node(default!);
        _head = dummy;
        _tail = dummy;
        _count = 0;
    }

    public int Count => _count;
    public bool IsEmpty => Count == 0;

    public void Enqueue(T item)
    {
        var newNode = new Node(item);

        while (true)
        {
            var tail = _tail;
            var next = tail.Next;

            // Check if tail is still consistent
            if (tail != _tail) continue;

            if (next == null)
            {
                // Try to link the new node
                if (Interlocked.CompareExchange(ref tail.Next, newNode, null) != null)
                    continue;

                // Try to swing tail to the new node
                Interlocked.CompareExchange(ref _tail, newNode, tail);
                Interlocked.Increment(ref _count);
                return;
            }

            // Tail was not pointing to the last node, try to advance
            Interlocked.CompareExchange(ref _tail, next, tail);
        }
    }

    public bool TryDequeue(out T result)
    {
        while (true)
        {
            var head = _head;
            var tail = _tail;
            var next = head.Next;

            // Check if head is still consistent
            if (head != _head) continue;

            // Is queue empty or tail falling behind?
            if (head == tail)
            {
                if (next == null)
                {
                    result = default!;
                    return false;
                }

                // Tail is falling behind, try to advance it
                Interlocked.CompareExchange(ref _tail, next, tail);
            }
            else
            {
                // Read value before CAS to prevent another thread from freeing next
                result = next!.Value;

                // Try to move head forward
                if (Interlocked.CompareExchange(ref _head, next, head) != head) 
                    continue;

                Interlocked.Decrement(ref _count);
                return true;
            }
        }
    }

    public bool TryPeek(out T? result)
    {
        var next = _head.Next;

        if (next == null)
        {
            result = default;
            return false;
        }

        result = next.Value;
        return true;
    }
}