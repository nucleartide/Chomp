#pragma once

#include <queue>

#include "CoreMinimal.h"

/**
 * PriorityQueue provides a simple wrapper around std::priority_queue so that I
 * only use the API methods I've studied the docs for.
 */
template <typename T, typename PriorityT>
struct TPriorityQueue
{
	typedef std::pair<PriorityT, T> Element;

	std::priority_queue<
		Element,				 // Type of stored element.
		std::vector<Element>,	 // Type of container used to store elements.
		std::greater<Element>> // The elements with greatest priority are ordered first, then we take the last element in this ordering.
		Elements;

	/**
	 * Clear the priority queue's elements.
	 */
	bool Empty() const
	{
		return Elements.empty();
	}

	void Put(T item, PriorityT priority)
	{
		// Using emplace avoids the necessity of having to copy/move a std::pair.
		// Instead, the std::pair is constructed within the .emplace() implementation.
		Elements.emplace(priority, item);
	}

	T Get()
	{
		T Best_Item = Elements.top().second; // Get the top element of the priority queue, and the underlying item within the pair.
		Elements.pop();						 // Pop off the top element.
		return Best_Item;					 // Return the top element.
	}
};
