import json
import matplotlib.pyplot as plt
import networkx as nx
from matplotlib.patches import Rectangle

# Load the graph data
with open('graph (4).json', 'r') as f:
    data = json.load(f)

# Create a directed graph
G = nx.Graph()

# Add nodes with attributes
for node in data['nodes']:
    G.add_node(node['id'], **node)

# Add edges with weights
for edge in data['edges']:
    G.add_edge(edge['source'], edge['target'], weight=edge['weight'])

# Create figure
fig, ax = plt.subplots(figsize=(20, 16))

# Define color mapping based on node type
color_map = {
    'entrance': '#FF6B6B',      # Red
    'hallway': '#4ECDC4',       # Teal
    'room': '#95E1D3',          # Light green
    'elevator': '#FFD93D',      # Yellow
    'stairs': '#FFA07A',        # Light orange
    'restroom': '#DDA0DD',      # Plum
    'outdoor': '#90EE90',       # Light green
    'building': '#FFB6C1',      # Light pink
    'classroom': '#87CEEB'      # Sky blue
}

# Check if nodes have X,Y coordinates
has_coordinates = all('x' in node and 'y' in node for node in data['nodes'])

if has_coordinates:
    print("✅ Using absolute coordinates from graph data")
    # Use the exact coordinates from the JSON (flip Y for matplotlib)
    pos = {node['id']: (node['x'], -node['y']) for node in data['nodes']}
    
    # Get image dimensions if available
    if 'metadata' in data and 'imageWidth' in data['metadata']:
        img_width = data['metadata']['imageWidth']
        img_height = data['metadata']['imageHeight']
        print(f"📐 Original image size: {img_width}x{img_height}px")
else:
    print("⚠️ No coordinates found, using spring layout")
    # Use spring layout for better visualization
    pos = nx.spring_layout(G, k=2, iterations=50, seed=42)

# Get node colors based on type
node_colors = [color_map.get(G.nodes[node].get('type', 'room'), '#95E1D3') for node in G.nodes()]

# Draw the graph
# Draw edges first (so they appear behind nodes)
nx.draw_networkx_edges(G, pos, alpha=0.3, width=1, edge_color='gray', ax=ax)

# Draw nodes
nx.draw_networkx_nodes(G, pos, node_color=node_colors, 
                       node_size=800, alpha=0.9, 
                       edgecolors='black', linewidths=2, ax=ax)

# Draw labels
nx.draw_networkx_labels(G, pos, font_size=10, font_weight='bold', ax=ax)

# Add legend
legend_elements = [
    plt.scatter([], [], c=color, s=100, label=type_name.capitalize(), edgecolors='black', linewidths=1)
    for type_name, color in color_map.items()
]
ax.legend(handles=legend_elements, loc='upper left', fontsize=10, framealpha=0.9)

# Add title and info
ax.set_title('Campus Map Graph Visualization (Preserving Original Layout)', fontsize=18, fontweight='bold', pad=20)
info_text = f'Nodes: {G.number_of_nodes()} | Edges: {G.number_of_edges()}'
if has_coordinates:
    info_text += ' | Using Absolute Coordinates ✓'
plt.text(0.5, 0.98, info_text, transform=fig.transFigure, 
         ha='center', fontsize=12, bbox=dict(boxstyle='round', facecolor='wheat', alpha=0.5))

ax.axis('equal')  # Keep aspect ratio
ax.axis('off')
plt.tight_layout()

# Save the figure
plt.savefig('campus_map_graph_preserved.png', dpi=300, bbox_inches='tight', facecolor='white')
print("\n✅ Graph visualization saved as 'campus_map_graph_preserved.png'")

# Also create a detailed version with edge weights
fig2, ax2 = plt.subplots(figsize=(24, 20))

# Draw edges with labels
nx.draw_networkx_edges(G, pos, alpha=0.2, width=2, edge_color='gray', ax=ax2)

# Draw nodes
nx.draw_networkx_nodes(G, pos, node_color=node_colors, 
                       node_size=1000, alpha=0.9, 
                       edgecolors='black', linewidths=2, ax=ax2)

# Draw labels
nx.draw_networkx_labels(G, pos, font_size=8, font_weight='bold', ax=ax2)

# Draw edge labels (weights)
edge_labels = nx.get_edge_attributes(G, 'weight')
nx.draw_networkx_edge_labels(G, pos, edge_labels, font_size=6, ax=ax2)

# Add legend
legend_elements = [
    plt.scatter([], [], c=color, s=150, label=type_name.capitalize(), edgecolors='black', linewidths=1)
    for type_name, color in color_map.items()
]
ax2.legend(handles=legend_elements, loc='upper left', fontsize=12, framealpha=0.9)

# Add title
ax2.set_title('Campus Map Graph - Detailed View with Original Layout', fontsize=20, fontweight='bold', pad=20)
ax2.axis('equal')
ax2.axis('off')

plt.tight_layout()
plt.savefig('campus_map_graph_detailed_preserved.png', dpi=300, bbox_inches='tight', facecolor='white')
print("✅ Detailed graph visualization saved as 'campus_map_graph_detailed_preserved.png'")

plt.show()

print(f"\n📊 Graph Statistics:")
print(f"   Total Nodes: {G.number_of_nodes()}")
print(f"   Total Edges: {G.number_of_edges()}")
print(f"   Graph Density: {nx.density(G):.4f}")
print(f"   Is Connected: {nx.is_connected(G)}")
if nx.is_connected(G):
    print(f"   Average Shortest Path: {nx.average_shortest_path_length(G, weight='weight'):.2f}")
print(f"   Average Degree: {sum(dict(G.degree()).values()) / G.number_of_nodes():.2f}")

if has_coordinates:
    print(f"\n✅ Layout Preserved: Your graph maintains the exact same layout as in the graph builder!")
