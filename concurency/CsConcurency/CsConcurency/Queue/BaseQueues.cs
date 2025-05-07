using System.Collections.Concurrent;

namespace CsConcurency.Queue;

public interface IQueue<T>
{
    void Enqueue(T item);
    bool TryDequeue(out T? obj);
    int Count { get; }
    bool IsEmpty { get; }
}

public class ConcurrentQueueWrapper<T> : ConcurrentQueue<T>, IQueue<T>;

