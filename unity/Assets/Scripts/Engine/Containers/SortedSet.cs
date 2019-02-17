using System.Collections;
using System.Collections.Generic;

// Poor man's SortedSet implementation using a SortedDictionary
public class SortedSet<T> : IEnumerable<T>, IEnumerable
{
    private SortedDictionary<T, bool> m_sortedList;

    public SortedSet(IComparer<T> comp)
    {
        m_sortedList = new SortedDictionary<T, bool>(comp);
    }
    
    public bool Contains(T key)
    {
        return m_sortedList.ContainsKey(key);
    }

    public bool Remove(T item)
    {
        return m_sortedList.Remove(item);
    }

    public void Add(T key)
    {
        m_sortedList.Add(key, true);
    }

    public bool Contains(object key)
    {
        return m_sortedList.ContainsKey((T)key);
    }

    public void Remove(object key)
    {
        m_sortedList.Remove((T) key);
    }

    public bool IsFixedSize
    {
        get { return false; }
    }

    IEnumerator<T> IEnumerable<T>.GetEnumerator()
    {
        return m_sortedList.Keys.GetEnumerator();
    }

    IEnumerator IEnumerable.GetEnumerator()
    {
        return m_sortedList.Keys.GetEnumerator();
    }
}
