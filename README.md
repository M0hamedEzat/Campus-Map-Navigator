# SSE-Map-Navigator

This project implements a campus map navigator using Dijkstra's algorithm to find the shortest path between buildings. The code is written in C++ and utilizes JSON for data representation.

## Data structures
- **Priority Queue**: A min-heap is used to efficiently retrieve the next node with the smallest distance.

- **Stack** : Used to reconstruct the path from the end node back to the start node.

- **Hash Table**: Used to store and retrieve nodes and their distances efficiently.

## Key Functions of Priority Queue
- `insert(node, distance)`: Inserts a node with its associated distance into the priority queue.
- `extractMin()`: Removes and returns the node with the smallest distance.
- `decreaseKey(node, newDistance)`: Updates the distance of a node in the
priority queue.
- `isEmpty()`: Checks if the priority queue is empty.

 - The private member functions include:
- `leftChild(index)`: Returns the index of the left child of a given node.
- `rightChild(index)`: Returns the index of the right child of a given node.
- `parent(index)`: Returns the index of the parent of a given node.
- `HpUp(index)`: Maintains the heap property by moving a node up the heap.
- `HpDown(index)`: Maintains the heap property by moving a node down the heap.

## key Functions of Hash Table
 - `insert(key, value)`: Inserts a key-value pair into the hash table.
 - `get(key)`: Retrieves the value associated with a given key.

### Hash Function
 - ``hash(key)``: Computes the hash value for a given key using the djb2 algorithm. 
 Uses Prime number 5381 and 33 for multiplication and addition respectively.