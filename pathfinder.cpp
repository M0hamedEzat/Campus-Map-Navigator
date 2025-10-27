#include <iostream>
#include <fstream>
#include <string>
#include <cstring>  
#include "json.hpp" //json lib to read json graph data file

using json = nlohmann::json;
using namespace std;

// Hash table
struct MapNode {
    char key[50];
    int value;
    MapNode* next;
    MapNode() : value(0), next(nullptr) { key[0] = '\0'; }
};

class StringIntMap {
private:
    MapNode** buckets; // pointer to an array of pointers 
    int numBuckets;
    // hash function 
    unsigned long hash(const char* str) {
        unsigned long hash = 5381;
        int c;
        while ((c = *str++)) {
            hash = ((hash << 5) + hash) + c; // left shift by 5 bits ( * (32 + 1))
        }
        return hash % numBuckets;
    }
public:
    StringIntMap(int size) : numBuckets(size) {
        buckets = new MapNode*[numBuckets];
        for (int i = 0; i < numBuckets; ++i) {
            buckets[i] = nullptr;
        }
    }
    ~StringIntMap() {
        for (int i = 0; i < numBuckets; ++i) {
            MapNode* entry = buckets[i];
            while (entry != nullptr) {
                MapNode* prev = entry;
                entry = entry->next;
                delete prev;
            }
        }
        delete[] buckets;
    }
    void insert(const char* key, int value) {
        unsigned long bucketIndex = hash(key);
        MapNode* entry = buckets[bucketIndex];
        while(entry != nullptr) {
            if(strcmp(entry->key, key) == 0) {
                entry->value = value;
                return;
            }
            entry = entry->next;
        }
        MapNode* newNode = new MapNode();
        strcpy(newNode->key, key);
        newNode->value = value;
        newNode->next = buckets[bucketIndex];
        buckets[bucketIndex] = newNode;
    }
    int get(const char* key) {
        unsigned long bucketIndex = hash(key);
        MapNode* entry = buckets[bucketIndex];
        while (entry != nullptr) {
            if (strcmp(entry->key, key) == 0) {
                return entry->value;
            }
            entry = entry->next;
        }
        return -1;
    }
};

// priority queue
struct HeapNode {
    int vertex;
    int distance;
};

class MinPriorityQueue {
private:
    HeapNode* heapArray; // dynamic array
    int capacity;
    int size;
    int parent(int i) { return (i - 1) / 2; }
    int left(int i) { return (2 * i + 1); }
    int right(int i) { return (2 * i + 2); }
    void swapNodes(int i, int j) {
        HeapNode temp = heapArray[i];
        heapArray[i] = heapArray[j];
        heapArray[j] = temp;
    }
    void siftDown(int i) {
        int l = left(i);
        int r = right(i);
        int smallest = i;
        if (l < size && heapArray[l].distance < heapArray[smallest].distance) {
            smallest = l;
        }
        if (r < size && heapArray[r].distance < heapArray[smallest].distance) {
            smallest = r;
        }
        if (smallest != i) {
            swapNodes(i, smallest);
            siftDown(smallest);
        }
    }
    void siftUp(int i) {
        while (i != 0 && heapArray[i].distance < heapArray[parent(i)].distance) {
            swapNodes(i, parent(i));
            i = parent(i);
        }
    }
public:
    MinPriorityQueue(int cap) : capacity(cap), size(0) {
        heapArray = new HeapNode[capacity];
    }
    ~MinPriorityQueue() {
        delete[] heapArray;
    }
    bool isEmpty() {
        return size == 0;
    }
    void insert(int vertex, int distance) {
        if (size == capacity) {
            cout << "Priority queue is full!" << endl;
            return;
        }
        size++;
        int i = size - 1;
        heapArray[i].vertex = vertex;
        heapArray[i].distance = distance;
        siftUp(i);
    }
    HeapNode extractMin() {
        if (isEmpty()) {
            return {-1, -1};
        }
        HeapNode root = heapArray[0];
        heapArray[0] = heapArray[size - 1];
        size--;
        siftDown(0);
        return root;
    }
};

// Stack for paths construction
struct StackNode {
    int vertex;
    StackNode* next;
};

class PathStack {
private:
    StackNode* top;
public:
    PathStack() : top(nullptr) {}
    ~PathStack() {
        while (!isEmpty()) {
            pop();
        }
    }
    bool isEmpty() {
        return top == nullptr;
    }
    void push(int vertex) {
        StackNode* newNode = new StackNode();
        newNode->vertex = vertex;
        newNode->next = top;
        top = newNode;
    }
    int pop() {
        if (isEmpty()) {
            return -1;
        }
        StackNode* temp = top;
        int vertex = temp->vertex;
        top = top->next;
        delete temp;
        return vertex;
    }
};


// graph implementation
struct AdjListNode {
    int destIndex; // index of the destination node
    int weight; // weight of the edge
    AdjListNode* next; // pointer to the next node
    AdjListNode() : destIndex(-1), weight(0), next(nullptr) {}
};

class ManualGraph {
public:
    int numVertices;
    AdjListNode** adjLists;
    StringIntMap* nodeMap;
    char** indexToName;
    int currentNodeIndex;

    ManualGraph(int vertices) : numVertices(vertices), currentNodeIndex(0) {
        adjLists = new AdjListNode*[numVertices];
        nodeMap = new StringIntMap(numVertices * 2);
        indexToName = new char*[numVertices];
        for (int i = 0; i < numVertices; ++i) {
            adjLists[i] = nullptr;
            indexToName[i] = new char[50];
            indexToName[i][0] = '\0';
        }
    }
    ~ManualGraph() {
        for (int i = 0; i < numVertices; ++i) {
            AdjListNode* current = adjLists[i];
            while (current != nullptr) {
                AdjListNode* temp = current;
                current = current->next;
                delete temp;
            }
            delete[] indexToName[i];
        }
        delete[] adjLists;
        delete[] indexToName;
        delete nodeMap;
    }
    void addNode(const char* name) {
        if (currentNodeIndex < numVertices && nodeMap->get(name) == -1) {
            nodeMap->insert(name, currentNodeIndex);
            strcpy(indexToName[currentNodeIndex], name);
            currentNodeIndex++;
        }
    }
    void addEdge(const char* srcName, const char* destName, int weight) {
        int srcIndex = nodeMap->get(srcName);
        int destIndex = nodeMap->get(destName);
        if (srcIndex == -1 || destIndex == -1) {
            return;
        }
        AdjListNode* newNode = new AdjListNode();
        newNode->destIndex = destIndex;
        newNode->weight = weight;
        newNode->next = adjLists[srcIndex];
        adjLists[srcIndex] = newNode;
    }
};



// list that stires doors variation
struct StringNode {
    char name[50];
    StringNode* next;
};

class StringList {
public:
    StringNode* head;
    int count;
    StringList() : head(nullptr), count(0) {}
    ~StringList() {
        StringNode* current = head;
        while (current != nullptr) {
            StringNode* temp = current;
            current = current->next;
            delete temp;
        }
    }
    void push(const char* name) {
        StringNode* newNode = new StringNode();
        strcpy(newNode->name, name);
        newNode->next = head;
        head = newNode;
        count++;
    }
};

StringList* findNodeVariations(ManualGraph* graph, const char* name) {
    StringList* list = new StringList();
    int nameLen = strlen(name);
    if (graph->nodeMap->get(name) != -1) {
        list->push(name);
        return list;
    }
    for (int i = 0; i < graph->numVertices; ++i) {
        const char* nodeName = graph->indexToName[i];
        if (strncmp(nodeName, name, nameLen) == 0) {
            int nodeNameLen = strlen(nodeName);
            if (nodeNameLen > nameLen && nodeNameLen <= nameLen + 2) { 
                list->push(nodeName);
            }
        }
    }
    return list;
}



// Dijkstra's Algorithm 

const int INF = 2147483647;

// This struct holds the result of a single Dijkstra run
struct PathResult {
    int distance;
    int* previous; // pointer to the 'previous' array
    int startIndex;
    int endIndex;

    PathResult() : distance(INF), previous(nullptr), startIndex(-1), endIndex(-1) {}
};

void dijkstra(ManualGraph* graph, int startIndex, int endIndex, PathResult& result) {
    int V = graph->numVertices;

    int* distances = new int[V];
    int* previous = new int[V]; // This will be stored in 'result'
    
    for (int i = 0; i < V; ++i) {
        distances[i] = INF;
        previous[i] = -1;
    }
    distances[startIndex] = 0;
    
    // 2. Create Priority Queue
    MinPriorityQueue pq(V * V); 
    pq.insert(startIndex, 0);

    // 3. Main Loop
    while (!pq.isEmpty()) {
        HeapNode minNode = pq.extractMin();
        int u = minNode.vertex;
        int u_dist = minNode.distance;

        if (u_dist > distances[u]) {
            continue;
        }
        
        // If we found the end node, we can stop.
        if (u == endIndex) {
            break;
        }

        // Loop over all neighbors
        AdjListNode* neighbor = graph->adjLists[u];
        while (neighbor != nullptr) {
            int v = neighbor->destIndex;
            int weight = neighbor->weight;
            int newDist = distances[u] + weight;
            if (newDist < distances[v]) {
                distances[v] = newDist;
                previous[v] = u;
                pq.insert(v, newDist);
            }
            neighbor = neighbor->next;
        }
    }

    // Store results in the struct
    result.distance = distances[endIndex];
    result.previous = previous; 
    result.startIndex = startIndex;
    result.endIndex = endIndex;

    delete[] distances;
}



int main() {
    // Load and Parse JSON file (this is by help of lib documentation and prevoius implementation) 
    const char* filename = "graph (4).json"; // Make sure this matches your file
    
    ifstream json_file(filename);
    if (!json_file.is_open()) {
        cerr << "Error: Could not open file '" << filename << "'" << endl;
        return 1;
    }
    json data;
    try {
        data = json::parse(json_file);
    } catch (json::parse_error& e) {
        cerr << "Error: Failed to parse JSON file." << endl;
        cerr << e.what() << endl;
        return 1;
    }
    json_file.close();

    // build graph
    int numNodes = data["nodes"].size();
    ManualGraph buildingGraph(numNodes);
    for (const auto& node : data["nodes"]) {
        string id = node["id"].get<string>();
        buildingGraph.addNode(id.c_str());
    }
    int edgeCount = 0;
    for (const auto& edge : data["edges"]) {
        string source = edge["source"].get<string>();
        string target = edge["target"].get<string>();
        int weight = edge["weight"].get<int>();
        buildingGraph.addEdge(source.c_str(), target.c_str(), weight);
        edgeCount++;
    }
    cout << "Graph '" << filename << "' loaded successfully." << endl;
    
    
    char startInput[50];
    char endInput[50];

    cout << "Enter Start Node (e.g., CP30 or E3): ";
    cin >> startInput;
    cout << "Enter End Node (e.g., CP32 or P061): ";
    cin >> endInput;
    
    // find node variations to solve doors problem
    StringList* startNodes = findNodeVariations(&buildingGraph, startInput);
    StringList* endNodes = findNodeVariations(&buildingGraph, endInput);

    if (startNodes->count == 0) {
        cout << "Error: Start node '" << startInput << "' not found." << endl;
        return 1;
    }
    if (endNodes->count == 0) {
        cout << "Error: End node '" << endInput << "' not found." << endl;
        return 1;
    }

    // Run Dijkstra for all combinations 
    
    PathResult bestResult; 

    // Loop for each start variation
    for (StringNode* start = startNodes->head; start != nullptr; start = start->next) {
        int startIndex = buildingGraph.nodeMap->get(start->name);
        if (startIndex == -1) continue;

        // Loop for each end variation
        for (StringNode* end = endNodes->head; end != nullptr; end = end->next) {
            int endIndex = buildingGraph.nodeMap->get(end->name);
            if (endIndex == -1) continue;

            PathResult currentResult;
            dijkstra(&buildingGraph, startIndex, endIndex, currentResult);

            if (currentResult.distance < bestResult.distance) {
                
                if (bestResult.previous != nullptr) {
                    delete[] bestResult.previous;
                }
                bestResult = currentResult; 
            } else {
                if (currentResult.previous != nullptr) {
                    delete[] currentResult.previous;
                }
            }
        }
    }
    
    if (bestResult.distance == INF || bestResult.previous == nullptr) {
        cout << "No path found from '" << startInput << "' to '" << endInput << "'." << endl;
    } else {
        cout << " Found " << endl;
        
        const char* bestStartName = buildingGraph.indexToName[bestResult.startIndex];
        const char* bestEndName = buildingGraph.indexToName[bestResult.endIndex];
        
        cout << "From: " << startInput << " (via " << bestStartName << ")" << endl;
        cout << "To:   " << endInput << " (via " << bestEndName << ")" << endl;
        cout << "Distance: " << bestResult.distance << endl;
        cout << "Path: " << endl;

        // Reconstruct path
        PathStack path;
        int current = bestResult.endIndex;
        while (current != -1) {
            path.push(current);
            if (current == bestResult.startIndex) break;
            current = bestResult.previous[current];
        }

        // Print path
        while (!path.isEmpty()) {
            int vertex = path.pop();
            cout << buildingGraph.indexToName[vertex];
            if (!path.isEmpty()) {
                cout << " -> ";
            }
        }
        cout << endl;
    }

            // Clean up
    delete startNodes;
    delete endNodes;
    if (bestResult.previous != nullptr) {
        delete[] bestResult.previous; 
    }

    return 0;
}