# Circuit Simplification and SAT Conversion Implementation
Implementation of logic circuit simplification using De Morgan's laws and conversion to CNF (Conjunctive Normal Form) using Tseitin transformation.

## 1. Data Structures Used

### Node Structure
- **Logical operation**: Type of gate ("AND", "OR", "NOT", or "" for leaf nodes)
- **Number of inputs**: How many inputs the node has
- **Input identifiers**: List of node identifiers that are inputs to this node
- **Node identifier**: Unique number identifying the node (node_number)

### Circuit Representation
- **circuit_nodes**: Total number of nodes in the circuit (increases with each new node created)
- **nr_inputs**: Number of input nodes (leaves) in the circuit
- **output_node_number**: Root node of the original circuit
- **Node array**: Nodes stored in array where index corresponds to node identifier

### Tseitin Variables
- **tseitin_root_number**: Root of the transformed circuit after Tseitin transformation
- **tseitin_nr_inputs**: Number of inputs in the transformed circuit

## 2. Implementation Logic

### Step 1: Reading and Building Circuit
**File parsing and circuit construction**:
- Reads input file containing circuit description
- Creates nodes for each gate (AND, OR, NOT) and leaf (input variable)
- Builds tree structure by storing node identifiers and their connections
- Identifies root node (output) and leaf nodes (inputs)

### Step 2: Circuit Simplification
**De Morgan's Laws** (applied recursively):
- **Double NOT elimination**: `NOT(NOT(a)) = a`
  - Two consecutive NOTs cancel each other
  - Input of second NOT becomes input to parent gate of first NOT
  - Updates circuit root if first NOT was the output
  
- **NOT-OR transformation**: `NOT(a OR b) = NOT(a) AND NOT(b)`
  - Creates new AND gate
  - Creates NOT gate for each input of OR gate
  - NOT gates become inputs to new AND gate
  - Replaces OR gate with AND gate in circuit
  
- **NOT-AND transformation**: `NOT(a AND b) = NOT(a) OR NOT(b)`
  - Creates new OR gate
  - Creates NOT gate for each input of AND gate
  - NOT gates become inputs to new OR gate
  - Replaces AND gate with OR gate in circuit

**Redundant Gate Elimination** (applied recursively):
- **Nested AND simplification**: `x1 AND (x2 AND x3 AND (x4 AND x5)) = x1 AND x2 AND x3 AND x4 AND x5`
  - Flattens nested AND gates into single multi-input AND gate
  
- **Nested OR simplification**: `x1 OR (x2 OR x3 OR (x4 OR x5)) = x1 OR x2 OR x3 OR x4 OR x5`
  - Flattens nested OR gates into single multi-input OR gate

### Step 3: Tseitin Transformation
**Converting to CNF for SAT solving**:
- Creates equisatisfiable circuit (satisfiability preserved)
- Creates new AND gate as root of transformed circuit
- Processes circuit from leaves to original root

**Transformation rules**:

**NOT gate** (`NOT(b)`): `NOT(b) = (NOT(a) OR NOT(b)) AND (a OR b)` where `a = NOT_gate_id`
- Creates two NOT gates for inputs
- Creates first OR gate with both NOT gates as inputs
- Creates second OR gate with original variables as inputs
- Both OR gates become inputs to root AND gate

**AND gate** (`b AND c`): `(b AND c) = (NOT(a) OR b) AND (NOT(a) OR c) AND (a OR NOT(b) OR NOT(c))` where `a = AND_gate_id`
- **Third clause**: Creates OR gate with NOT gates for each input of AND gate, plus gate identifier
- **First two clauses**: For each input, creates NOT gate for AND gate identifier, creates OR gate with NOT and current input
- All OR gates become inputs to root AND gate

**OR gate** (`b OR c`): `(b OR c) = (a OR NOT(b)) AND (a OR NOT(c)) AND (NOT(a) OR b OR c)` where `a = OR_gate_id`
- **Third clause**: Creates OR gate with NOT of gate identifier plus all inputs of OR gate
- **First two clauses**: For each input, creates NOT gate for that input, creates OR gate with NOT and gate identifier
- All OR gates become inputs to root AND gate

### Step 4: Output Generation
**Writing CNF to file**:
- Root is AND gate with OR gates as inputs (CNF format)
- For each OR clause (input to root AND):
  - **Leaf nodes**: Written directly as positive literals
  - **NOT gates**: Input written as negative literal (with minus sign)
  - **OR gates**: Recursively processes inputs (leaves or NOT gates)
