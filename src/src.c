#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct {
    int node_number;
    // = "" when we have input type node to differentiate from other nodes
    char *logical_operation;
    int nr_inputs;
    int *inputs; // index 0 - input 1, index 1 - input 2
} Node;

int circuit_nodes, nr_inputs, output_node_number, tseitin_root_number, tseitin_nr_inputs;
Node **nodes;  // index 1 - node 1, index 2 - node 2

// Returns the actual node found at input_index position in the input list of node_number
Node *get_input_node_of(int node_number, int input_index)
{
    int input_node_number = nodes[node_number]->inputs[input_index];
    return nodes[input_node_number];
}

// Create a new node with sent parameters and save it in the circuit nodes array
Node *create_node(int *inputs, char* logical_operation, int nr_inputs)
{
    Node *node = calloc(1, sizeof(Node));
    node->logical_operation = calloc(strlen(logical_operation) + 1, sizeof(char));
    strcpy(node->logical_operation, logical_operation);

    node->nr_inputs = nr_inputs;
    node->inputs = calloc(nr_inputs, sizeof(int));
    for (int i = 0; i < nr_inputs; i++)
        node->inputs[i] = inputs[i];
    
    circuit_nodes++;
    nodes = realloc(nodes, (circuit_nodes + 1) * sizeof(Node *));
    nodes[circuit_nodes] = node;
    node->node_number = circuit_nodes;
    
    return node;
}

// Add the sent node to the input list of node_number, if it doesn't already exist
void add_input_to_node(int input_node_number, int node_number) 
{
    for (int index = 0; index < nodes[node_number]->nr_inputs; index++) {
        if (nodes[node_number]->inputs[index] == input_node_number)
            return;
    }

    nodes[node_number]->nr_inputs++;
    nodes[node_number]->inputs = realloc(nodes[node_number]->inputs, nodes[node_number]->nr_inputs * sizeof(int));
    int index_input_node_number = nodes[node_number]->nr_inputs;
    nodes[node_number]->inputs[index_input_node_number - 1] = input_node_number;
}

// Remove the node from the input list of node_number, if it exists
void remove_input_from_node(int input_node_number, int node_number) 
{
    int index_input_to_remove = -1;
    for (int index = 0; index < nodes[node_number]->nr_inputs; index++) {
        if (nodes[node_number]->inputs[index] == input_node_number)
            index_input_to_remove = index;
    }
    
    if (index_input_to_remove == -1)
        return;

    for (int index = index_input_to_remove; index < nodes[node_number]->nr_inputs - 1; index++) {
        nodes[node_number]->inputs[index] = nodes[node_number]->inputs[index + 1];
    }
    
    nodes[node_number]->nr_inputs--;
    nodes[node_number]->inputs = realloc(nodes[node_number]->inputs, nodes[node_number]->nr_inputs * sizeof(int));
}

// Process data from input file, building the circuit based on it
void process_input(int nr_lines, char **input, int nr_inputs)
{                                         
    for (int i = 0; i < nr_inputs; i++)
       create_node(NULL, "", 0);

    for (int j = 0; j <= nr_lines - 1; j++) {
        int node_inputs[circuit_nodes], nr_inputs = 0;
        char *token = strtok(input[j], " ");
        char *logical_operation = calloc(strlen(token) + 1, sizeof(char));
        strcpy(logical_operation, token);

        while (token != NULL) {
            char *token = strtok(NULL, " ");
            if (token == NULL)
                break;
            int input_node_number = atoi(token);
            node_inputs[nr_inputs++] = input_node_number;
        }
        
        int node_number = node_inputs[nr_inputs - 1];
        nr_inputs--;

        create_node(node_inputs, logical_operation, nr_inputs);
        free(logical_operation);
    }
}

// Read input file line by line and save everything in a matrix
char **read_input_file(FILE *input_file, int *nr_lines)
{
    char **input = NULL;
    char *line_to_read = NULL;
    size_t len_line = 0;

    ssize_t read = getline(&line_to_read, &len_line, input_file);
    while (read != -1) {
        // line_to_read -- string of char* type elements
        if (line_to_read[strlen(line_to_read) - 1] == '\n')
            line_to_read[strlen(line_to_read) - 1] = '\0';
        
        (*nr_lines)++;

        input = realloc(input, *nr_lines * sizeof(char*));
        input[*nr_lines - 1] = calloc(strlen(line_to_read) + 1, sizeof(char));

        strcpy(input[*nr_lines - 1], line_to_read);
        read = getline(&line_to_read, &len_line, input_file);
    }
    
    free(line_to_read);
    return input;
}

// Open input file, read line by line and build the circuit
void parse_input(char *input)
{
    FILE *input_file = fopen(input, "r");
    fscanf(input_file, "%d %d\n", &nr_inputs, &output_node_number);
    
    // Read by lines (array of lines)
    int nr_lines = 0;
    char **data = read_input_file(input_file, &nr_lines);
    process_input(nr_lines, data, nr_inputs);
    
    for (int index = 0; index < nr_lines; index++)
        free(data[index]);
    free(data);
    fclose(input_file);
}

// Modify current tree root with another root
void change_output_node_number(int new_output_node_number) 
{
    output_node_number = new_output_node_number;
}

// De Morgan's laws 2 and 3
int split_complex_gate(int node_number, int parent_node_number, char *logical_operation)
{
    if (parent_node_number > 0)
        remove_input_from_node(node_number, parent_node_number);

    // Create an AND gate if I have the OR law or OR if the law is with AND
    Node *new_node = create_node(NULL, logical_operation, 0);
    // The new gate built will be added as input for the NOT gate output
    if (parent_node_number > 0)
        add_input_to_node(new_node->node_number, parent_node_number);
    
    // For laws 2 and 3 I build a NOT gate for each input which I add to the circuit:
    // I add the current input to a NOT gate
    // I link the NOT gate to the AND/OR gate created earlier
    Node *input = get_input_node_of(node_number, 0);
    for (int index = 0; index < input->nr_inputs; index++) {
        Node *not_gate = create_node(NULL, "NOT", 0);
        add_input_to_node(not_gate->node_number, new_node->node_number);
        add_input_to_node(input->inputs[index], not_gate->node_number);
    }

    // No modification of circuit root (output)
    if (parent_node_number > 0)
        return 2;

    // Modification of circuit root (output)
    change_output_node_number(new_node->node_number);
    return 1;
}

// Apply De Morgan's laws to the entire circuit
int apply_de_morgan_laws(int node_number, int parent_node_number)
{
    Node *input = get_input_node_of(node_number, 0);
    if (strcmp(input->logical_operation, "") == 0)  // Circuit leaf
        return 0;
    
    // Law 1 -> 2 negations cancel each other out
    if (strcmp(input->logical_operation, "NOT") == 0) {
        Node *input_not_gate = get_input_node_of(input->node_number, 0);
        if (parent_node_number > 0) {
            add_input_to_node(input_not_gate->node_number, parent_node_number);
            remove_input_from_node(node_number, parent_node_number);
            return 2;  // Root was not modified
        }
        
        // If the circuit root (output) is a NOT gate and then we have another NOT =>
        // the tree root must be modified with the first input of the second NOT
        change_output_node_number(input_not_gate->node_number);
        return 1;  // Root was modified
    }

    // Law 2: !(a && b) => !a || !b
    if (strcmp(input->logical_operation, "AND") == 0)
        return split_complex_gate(node_number, parent_node_number, "OR");  

    // Law 3: !(a || b) => !a && !b
    return split_complex_gate(node_number, parent_node_number, "AND");  
}

/*
x1 && (x2 && x3 && (x4 && x5)) = x1 && (x2 && x3 && x4 && x5) = x1 && x2 && x3 && x4 && x5
x1 || (x2 || x3 || (x4 || x5)) = x1 || (x2 || x3 || x4 || x5) = x1 || x2 || x3 || x4 || x5
 */
void remove_unnecessary_nodes(int node_number)
{
    int index = 0;
    while (index < nodes[node_number]->nr_inputs) {
        Node *input = get_input_node_of(node_number, index);
        if (strcmp(input->logical_operation, nodes[node_number]->logical_operation) != 0) {
            index++;
            continue;
        }

        remove_input_from_node(input->node_number, node_number);
        for (int i = 0; i < input->nr_inputs; i++)
            add_input_to_node(input->inputs[i], node_number);
    }
}

// Call De Morgan's laws and remove redundant gates
int simplify_circuit(int node_number, int parent_node_number)
{
    // Leaf => stop recursion
    if (strcmp(nodes[node_number]->logical_operation, "") == 0)
        return 0;

    // De Morgan's laws for root
    int return_value = 0;
    if (strcmp(nodes[node_number]->logical_operation, "NOT") == 0) {
        int result = apply_de_morgan_laws(node_number, parent_node_number);
        if (result == 1) {
            // Root was modified => apply recursion once more
            simplify_circuit(output_node_number, 0);
            return 0;
        }
        if (result == 2)
            return_value = 1;
    }

    // Recursion continues by calling the function for current node inputs
    int index = 0;
    while (index < nodes[node_number]->nr_inputs) {
        int inputs_modified = simplify_circuit(nodes[node_number]->inputs[index], node_number);
        /* Only if I haven't modified a gate do I advance, because otherwise I have a gate
         * that I haven't analyzed yet and which I could skip over
         */
        if (!inputs_modified)
            index++;
    }

    // Remove redundant gates
    remove_unnecessary_nodes(node_number);
    return return_value;
}

/* NOT gate Tseitin transform => 
 * !b => (!a || !b) && (a || b), a = NOT gate number
 */
void transform_not_gate(int node_number)
{
    // Create a NOT gate for node_number
    Node *not_gate_node = create_node(NULL, "NOT", 0);
    add_input_to_node(node_number, not_gate_node->node_number);
    
    // Create a NOT gate for the NOT gate input
    Node *not_gate_input_node = create_node(NULL, "NOT", 0);
    add_input_to_node(nodes[node_number]->inputs[0], not_gate_input_node->node_number);

    /* (!a || !b)
     * Create OR gate
     * Add the two NOTs as inputs for it
     */
    Node *or_gate_negations = create_node(NULL, "OR", 0);
    add_input_to_node(not_gate_node->node_number, or_gate_negations->node_number);
    add_input_to_node(not_gate_input_node->node_number, or_gate_negations->node_number);

    /* (a || b)
     * Create OR gate
     * Add the two NOTs as inputs for it
     */
    Node *or_gate = create_node(NULL, "OR", 0);
    add_input_to_node(node_number, or_gate->node_number);
    add_input_to_node(nodes[node_number]->inputs[0], or_gate->node_number);

    // Add the two ORs to the Tseitin circuit root (AND)
    add_input_to_node(or_gate->node_number, tseitin_root_number);
    add_input_to_node(or_gate_negations->node_number, tseitin_root_number);

    // Modify node_number and input so they become leaves in circuit
    strcpy(nodes[node_number]->logical_operation, "");
    strcpy(get_input_node_of(node_number, 0)->logical_operation, "");
}

/* AND gate - Tseitin
 * (b && c) = (!a || b) && (!a || c) && (a || !b || !c), a = AND gate number
 */
void transform_and_gate(int node_number)
{
    // OR for last parenthesis
    Node *or_gate = create_node(NULL, "OR", 0);

    // Add OR gate to circuit root
    add_input_to_node(or_gate->node_number, tseitin_root_number);

    // Add current node (a)
    add_input_to_node(node_number, or_gate->node_number);
    
    // For each input of a create a new NOT gate that becomes input for gate a
    for (int index = 0; index < nodes[node_number]->nr_inputs; index++) {
        Node *not_gate = create_node(NULL, "NOT", 0);
        add_input_to_node(not_gate->node_number, or_gate->node_number);
        add_input_to_node(nodes[node_number]->inputs[index], not_gate->node_number);
    }

    // First parentheses
    for (int index = 0; index < nodes[node_number]->nr_inputs; index++) {
        // Create !a
        Node *not_gate_input = create_node(NULL, "NOT", 0);
        add_input_to_node(node_number, not_gate_input->node_number);

        // Add inputs b and c as inputs of OR gate
        Node *or_gate_input = create_node(NULL, "OR", 0);
        add_input_to_node(not_gate_input->node_number, or_gate_input->node_number);
        add_input_to_node(nodes[node_number]->inputs[index], or_gate_input->node_number);

        // Add each newly formed clause to Tseitin circuit root
        add_input_to_node(or_gate_input->node_number, tseitin_root_number);

        // Processed nodes become leaves
        strcpy(get_input_node_of(node_number, index)->logical_operation, "");
    }
    
    // Current node becomes leaf
    strcpy(nodes[node_number]->logical_operation, "");
}

/* Analogous to case above, but here we have:
 * (b || c) => (a || !b) && (a || !c) && (!a || b || c), where a = OR gate number
 */
void transform_or_gate(int node_number)
{
    Node *or_gate = create_node(NULL, "OR", 0);
    add_input_to_node(or_gate->node_number, tseitin_root_number);

    Node *not_gate = create_node(NULL, "NOT", 0);
    add_input_to_node(not_gate->node_number, or_gate->node_number);
    add_input_to_node(node_number, not_gate->node_number);

    for (int index = 0; index < nodes[node_number]->nr_inputs; index++)
        add_input_to_node(nodes[node_number]->inputs[index], or_gate->node_number);

    for (int index = 0; index < nodes[node_number]->nr_inputs; index++) {
        Node *not_gate_input = create_node(NULL, "NOT", 0);
        add_input_to_node(nodes[node_number]->inputs[index], not_gate_input->node_number);

        Node *or_gate_input = create_node(NULL, "OR", 0);
        add_input_to_node(node_number, or_gate_input->node_number);
        add_input_to_node(not_gate_input->node_number, or_gate_input->node_number);

        add_input_to_node(or_gate_input->node_number, tseitin_root_number);
        strcpy(get_input_node_of(node_number, index)->logical_operation, "");
    }

    strcpy(nodes[node_number]->logical_operation, "");
}

// Call one of the above functions depending on current node gate type
void transform_node(int node_number)
{
    if (strcmp(nodes[node_number]->logical_operation, "NOT") == 0) {
        transform_not_gate(node_number);
        return;
    }

    if (strcmp(nodes[node_number]->logical_operation, "AND") == 0) {
        transform_and_gate(node_number);
        return;
    }

    transform_or_gate(node_number);
}

// Recursive function for transforming circuit into a new circuit
void recursive_tseitin(int node_number)
{
    // It's a leaf
    if (strcmp(nodes[node_number]->logical_operation, "") == 0)
        return;

    // Traversal from leaves to root
    for (int index = 0; index < nodes[node_number]->nr_inputs; index++)
        recursive_tseitin(nodes[node_number]->inputs[index]);
    
    // Call transformation for current node
    transform_node(node_number);
}

// Tseitin transformation
int tseitin_transform(int node_number)
{
    // Keep initial number of nodes for file display
    tseitin_nr_inputs = circuit_nodes;

    // Create new root - AND
    Node *tseitin_root = create_node(NULL, "AND", 0);
    tseitin_root_number = tseitin_root->node_number;

    // Process root
    recursive_tseitin(node_number);

    // Add new clause formed to AND root and current node becomes leaf in circuit
    add_input_to_node(node_number, tseitin_root_number);
    strcpy(nodes[node_number]->logical_operation, "");
}

// Display in output file according to requirement
void dimacs(char *output_file)
{
    FILE *output = fopen(output_file, "w+");
    fprintf(output, "p cnf %d %d\n", tseitin_nr_inputs, nodes[tseitin_root_number]->nr_inputs);
    
    // Traverse inputs
    for (int index = 0; index < nodes[tseitin_root_number]->nr_inputs; index++) {
        Node *input_node = get_input_node_of(tseitin_root_number, index);
        
        // If we have a leaf => direct display
        if (strcmp(input_node->logical_operation, "") == 0) {
            fprintf(output, "%d 0\n", input_node->node_number);
            continue;
        }

        // Otherwise, we have an OR gate => traverse all inputs
        int current_node = nodes[tseitin_root_number]->inputs[index]; 
        for (int index1 = 0; index1 < nodes[current_node]->nr_inputs; index1++) {
            Node *input_current_node = get_input_node_of(current_node, index1);
            
            // It's a leaf => display
            if (strcmp(input_current_node->logical_operation, "") == 0)
                fprintf(output, "%d ", input_current_node->node_number);
            // NOT type input => display node number with minus in front
            else
                fprintf(output, "-%d ", input_current_node->inputs[0]);
        }
        fprintf(output, "0\n");
    }

    fclose(output);
}

// Deallocate allocated memory
void deallocate_memory(void)
{
    for (int index = 0; index < circuit_nodes; index++) {
        free(nodes[index + 1]->logical_operation);
        free(nodes[index + 1]->inputs);
        free(nodes[index + 1]);
    }
    free(nodes);
}

int main(int nr_args, char *args[])
{
    // Process input data
    parse_input(args[1]);

    // Simplify circuit by applying De Morgan's laws and removing redundant gates
    simplify_circuit(output_node_number, 0);

    // Apply Tseitin transformation
    tseitin_transform(output_node_number);

    // Display in file
    dimacs(args[2]);

    // Deallocate memory
    deallocate_memory();
    return 0;
}
