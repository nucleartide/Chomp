#pragma once

#include <queue>

#include "CoreMinimal.h"

/**
 * PriorityQueue provides a simple wrapper around std::priority_queue so that I
 * only use the API methods I've studied the docs for.
 */
template <typename T, typename priority_t>
struct PriorityQueue
{
	typedef std::pair<priority_t, T> PQElement;

	std::priority_queue<
		PQElement,				 // Type of stored element.
		std::vector<PQElement>,	 // Type of container used to store elements.
		std::greater<PQElement>> // The elements with greatest priority are ordered first, then we take the last element in this ordering.
		elements;

	/**
	 * Clear the priority queue's elements.
	 */
	bool empty() const
	{
		return elements.empty();
	}

	void put(T item, priority_t priority)
	{
		// Using emplace avoids the necessity of having to copy/move a std::pair.
		// Instead, the std::pair is constructed within the .emplace() implementation.
		elements.emplace(priority, item);
	}

	T get()
	{
		T best_item = elements.top().second; // Get the top element of the priority queue, and the underlying item within the pair.
		elements.pop();						 // Pop off the top element.
		return best_item;					 // Return the top element.
	}
};
