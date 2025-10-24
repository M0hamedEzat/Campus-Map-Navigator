#include <iostream>
#include <fstream> // For std::ifstream (file reading)
#include <string>  // For std::string (used by the JSON parser)
#include "json.hpp" // The JSON library you downloaded

// Use the nlohmann::json library
using json = nlohmann::json;

// --- 1. Manual C-String Helper Functions ---
// (Replicating functionality from <cstring>)

int my_strlen(const char* s) {
    int len = 0;
    while (s[len] != '\0') {
        len++;
    }
    return len;
}

int my_strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

char* my_strcpy(char* dest, const char* src) {
    char* original_dest = dest;
    while (*src != '\0') {
        *dest = *src;
        dest++;
        src++;
    }
    *dest = '\0';
    return original_dest;
}

// *** NEW ***
// Manual strncmp (compares first 'n' characters)
int my_strncmp(const char* s1, const char* s2, int n) {
    while (n && *s1 && (*s1 == *s2)) {
        s1++;
        s2++;
        n--;
    }
    if (n == 0) {
        return 0; // Reached 'n' chars, all equal
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}


// --- 2. Manual Hash Map (String -> Int) ---
// (This class is unchanged)
struct MapNode {
    char key[50];
    int value;
    MapNode* next;
    MapNode() : value(0), next(nullptr) { key[0] = '\0'; }
};

class StringIntMap {
private:
    MapNode** buckets;
    int numBuckets;
    unsigned long hash(const char* str) {
        unsigned long hash = 5381;
        int c;
        while ((c = *str++)) {
            hash = ((hash << 5) + hash) + c;
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
            if(my_strcmp(entry->key, key) == 0) {
                entry->value = value;
                return;
            }
            entry = entry->next;
        }
        MapNode* newNode = new MapNode();
        my_strcpy(newNode->key, key);
        newNode->value = value;
        newNode->next = buckets[bucketIndex];
        buckets[bucketIndex] = newNode;
    }
    int get(const char* key) {
        unsigned long bucketIndex = hash(key);
        MapNode* entry = buckets[bucketIndex];
        while (entry != nullptr) {
            if (my_strcmp(entry->key, key) == 0) {
                return entry->value;
            }
            entry = entry->next;
        }
        return -1;
    }
};

// --- 3. Manual Graph (Adjacency List) ---
// (This class is unchanged)
struct AdjListNode {
    int destIndex;
    int weight;
    AdjListNode* next;
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
            my_strcpy(indexToName[currentNodeIndex], name);
            currentNodeIndex++;
        }
    }
    void addEdge(const char* srcName, const char* destName, int weight) {
        int srcIndex = nodeMap->get(srcName);
        int destIndex = nodeMap->get(destName);
        if (srcIndex == -1 || destIndex == -1) {
            // This is noisy, let's quiet it
            // std::cout << "Error: Node not found for edge " << srcName << " -> " << destName << std::endl;
            return;
        }
        AdjListNode* newNode = new AdjListNode();
        newNode->destIndex = destIndex;
        newNode->weight = weight;
        newNode->next = adjLists[srcIndex];
        adjLists[srcIndex] = newNode;
    }
};

// --- 4. Manual Min-Priority Queue (Min-Heap) ---
// (This class is unchanged)
struct HeapNode {
    int vertex;
    int distance;
};

class MinPriorityQueue {
private:
    HeapNode* heapArray;
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
            std::cout << "Priority queue is full!" << std::endl;
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

// --- 5. Manual Stack (for Path Reconstruction) ---
// (This class is unchanged)
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

// --- 6. Dijkstra's Algorithm Implementation ---

const int INF = 2147483647;

// *** MODIFIED FUNCTION ***
// This function no longer takes an end node.
// It calculates distances from 'startIndex' to ALL other nodes.
// It allocates and returns the 'distances' and 'previous' arrays
// via pointer references. The CALLER must delete them.
void dijkstra(ManualGraph* graph, int startIndex, int*& distances, int*& previous) {
    int V = graph->numVertices;

    // 1. Allocate and initialize arrays
    distances = new int[V];
    previous = new int[V];
    for (int i = 0; i < V; ++i) {
        distances[i] = INF;
        previous[i] = -1;
    }
    distances[startIndex] = 0;
    
    // 2. Create Priority Queue
    MinPriorityQueue pq(V * V); // Safe overestimate
    pq.insert(startIndex, 0);

    // 3. Main Loop
    while (!pq.isEmpty()) {
        HeapNode minNode = pq.extractMin();
        int u = minNode.vertex;
        int u_dist = minNode.distance;

        if (u_dist > distances[u]) {
            continue;
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
    // We are done. The 'distances' and 'previous' arrays are complete.
}

// --- 7. *** NEW *** Helper Classes/Functions for Node Variations ---

// A simple linked list for storing strings
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
        my_strcpy(newNode->name, name);
        newNode->next = head;
        head = newNode;
        count++;
    }
};

// Finds all specific node IDs that match a generic name
// e.g., "CP30" -> finds "CP30a", "CP30b"
// e.g., "E3" -> finds "E3"
StringList* findNodeVariations(ManualGraph* graph, const char* name) {
    StringList* list = new StringList();
    int nameLen = my_strlen(name);

    // 1. Check for an exact match first.
    if (graph->nodeMap->get(name) != -1) {
        list->push(name);
        return list;
    }

    // 2. If no exact match, check for partial matches (e.g., "CP30a")
    for (int i = 0; i < graph->numVertices; ++i) {
        const char* nodeName = graph->indexToName[i];
        
        // Check if nodeName starts with name
        if (my_strncmp(nodeName, name, nameLen) == 0) {
            // Check if it's a "door" (e.g., "CP30" + "a" = "CP30a")
            int nodeNameLen = my_strlen(nodeName);
            if (nodeNameLen > nameLen && nodeNameLen <= nameLen + 2) { // Allows for "a", "b", or even "10"
                // This logic can be tighter, but for 'a'/'b' it's fine
                list->push(nodeName);
            }
        }
    }
    return list;
}


// --- 8. Main Function (Modified for New Logic) ---

int main(int argc, char* argv[]) {
    // --- 1. Load and Parse JSON file ---
    const char* filename = "graph (4).json"; // Make sure this matches your file
    std::ifstream json_file(filename);
    if (!json_file.is_open()) {
        std::cerr << "Error: Could not open file '" << filename << "'" << std::endl;
        return 1;
    }

    json data;
    try {
        data = json::parse(json_file);
    } catch (json::parse_error& e) {
        std::cerr << "Error: Failed to parse JSON file." << std::endl;
        std::cerr << e.what() << std::endl;
        return 1;
    }
    json_file.close();

    // --- 2. Build the ManualGraph ---
    int numNodes = data["nodes"].size();
    if (numNodes == 0) {
        std::cerr << "Error: No nodes found in JSON file." << std::endl;
        return 1;
    }
    ManualGraph buildingGraph(numNodes);
    for (const auto& node : data["nodes"]) {
        std::string id = node["id"].get<std::string>();
        buildingGraph.addNode(id.c_str());
    }
    std::cout << "Successfully loaded " << buildingGraph.currentNodeIndex << " nodes." << std::endl;
    int edgeCount = 0;
    for (const auto& edge : data["edges"]) {
        std::string source = edge["source"].get<std::string>();
        std::string target = edge["target"].get<std::string>();
        int weight = edge["weight"].get<int>();
        buildingGraph.addEdge(source.c_str(), target.c_str(), weight);
        edgeCount++;
    }
    std::cout << "Successfully loaded " << edgeCount << " edge entries." << std::endl;
    
    
    // --- 3. Get Command-Line Arguments ---
    if (argc != 3) {
        // Print to cerr (error stream) so it doesn't interfere with server output
        std::cerr << "Usage: ./pathfinder [start_node] [end_node]" << std::endl;
        std::cerr << "Example: ./pathfinder CP30 CP32" << std::endl;
        return 1;
    }
    const char* startArg = argv[1];
    const char* endArg = argv[2];


    // --- 4. *** NEW LOGIC *** Find Best Path ---
    
    StringList* startNodes = findNodeVariations(&buildingGraph, startArg);
    StringList* endNodes = findNodeVariations(&buildingGraph, endArg);

    if (startNodes->count == 0) {
        std::cerr << "Error: Start node '" << startArg << "' not found." << std::endl;
        return 1;
    }
    if (endNodes->count == 0) {
        std::cerr << "Error: End node '" << endArg << "' not found." << std::endl;
        return 1;
    }

    int bestDistance = INF;
    int bestStartIndex = -1;
    int bestEndIndex = -1;
    int* bestPrevious = nullptr;
    const char* bestStartName = "";
    const char* bestEndName = "";

    // Loop for each *start* variation (e.g., "CP30a", "CP30b")
    for (StringNode* start = startNodes->head; start != nullptr; start = start->next) {
        int startIndex = buildingGraph.nodeMap->get(start->name);
        if (startIndex == -1) continue;

        int* distances = nullptr;
        int* previous = nullptr;
        
        // Run Dijkstra ONCE for this start node
        dijkstra(&buildingGraph, startIndex, distances, previous);

        // Check the distance to each *end* variation
        for (StringNode* end = endNodes->head; end != nullptr; end = end->next) {
            int endIndex = buildingGraph.nodeMap->get(end->name);
            if (endIndex == -1) continue;

            if (distances[endIndex] < bestDistance) {
                bestDistance = distances[endIndex];
                bestStartIndex = startIndex;
                bestEndIndex = endIndex;
                bestStartName = start->name; // Store the name
                bestEndName = end->name;     // Store the name

                // Save this 'previous' array as the new best
                if (bestPrevious) {
                    delete[] bestPrevious;
                }
                bestPrevious = previous;
                previous = nullptr; // Mark as "moved" so it's not deleted below
            }
        }

        // Clean up arrays from this Dijkstra run
        delete[] distances;
        if (previous) { // Delete 'previous' if it wasn't the best
            delete[] previous;
        }
    }

    // --- 5. Print the Single Best Result ---
    // This output is designed to be read by your server script
    
    if (bestDistance == INF) {
        std::cerr << "No path found from '" << startArg << "' to '" << endArg << "'." << std::endl;
    } else {
        // We have a winner. Reconstruct the path.
        PathStack path;
        int current = bestEndIndex;
        while (current != -1) {
            path.push(current);
            if (current == bestStartIndex) break; // Found start
            current = bestPrevious[current];
        }

        // Print the path in "A -> B -> C" format
        while (!path.isEmpty()) {
            int vertex = path.pop();
            std::cout << buildingGraph.indexToName[vertex];
            if (!path.isEmpty()) {
                std::cout << " -> ";
            }
        }
        std::cout << std::endl;
        
        // --- Optional: Print human-readable summary to cerr ---
        std::cerr << "--- Shortest Path Found ---" << std::endl;
        std::cerr << "From: " << startArg << " (via " << bestStartName << ")" << std::endl;
        std::cerr << "To:   " << endArg << " (via " << bestEndName << ")" << std::endl;
        std::cerr << "Distance: " << bestDistance << std::endl;
        std::cerr << "---------------------------" << std::endl;
    }

    // --- 6. Final Cleanup ---
    delete startNodes;
    delete endNodes;
    if (bestPrevious) {
        delete[] bestPrevious;
    }

    return 0;
}