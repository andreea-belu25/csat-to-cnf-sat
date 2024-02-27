#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct {
    int node_number;
    // = "" cand avem nod de tip input pentru a diferentia de restul nodurilor
    char *logical_operation;
    int nr_inputs;
    int *inputs; // index 0 - input 1, index 1 - input 2
} Node;

int circuit_nodes, nr_inputs, output_node_number, tseitin_root_number, tseitin_nr_inputs;
Node **nodes;  // index 1 - node 1, index 2 - node 2

//  returneaza nodul propriu-zis ce se afla pe pozitia input_index in lista de inputuri a lui node_number
Node *get_input_node_of(int node_number, int input_index)
{
    int input_node_number = nodes[node_number]->inputs[input_index];
    return nodes[input_node_number];
}

//  creez un nod nou cu parametrii trimisi si il salvez in array-ul cu nodurile circuitului
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

//  adaug nodul trimis in lista de inputs a nodului node_number, daca nu exista deja
void add_input_to_node(int input_node_number, int node_number) 
{
    for (int index = 0; index < nodes[node_number]->nr_inputs; index++)
        if (nodes[node_number]->inputs[index] == input_node_number)
            return;

    nodes[node_number]->nr_inputs++;
    nodes[node_number]->inputs = realloc(nodes[node_number]->inputs, nodes[node_number]->nr_inputs * sizeof(int));
    int index_input_node_number = nodes[node_number]->nr_inputs;
    nodes[node_number]->inputs[index_input_node_number - 1] = input_node_number;
}

//  sterg nodul din lista de inputs a nodului node_number, daca acesta exista
void remove_input_from_node(int input_node_number, int node_number) 
{
    int index_input_to_remove = -1;
    for (int index = 0; index < nodes[node_number]->nr_inputs; index++)
        if (nodes[node_number]->inputs[index] == input_node_number)
            index_input_to_remove = index;
    
    if (index_input_to_remove == -1)
        return;

    for (int index = index_input_to_remove; index < nodes[node_number]->nr_inputs - 1; index++) {
        nodes[node_number]->inputs[index] = nodes[node_number]->inputs[index + 1];
    }
    nodes[node_number]->nr_inputs--;
    nodes[node_number]->inputs = realloc(nodes[node_number]->inputs, nodes[node_number]->nr_inputs * sizeof(int));
}

//  proceseaza datele din fisierul de input, construind circuitul pe baza acestuia
void process_input(int nr_lines, char **input, int nr_inputs)
{                                         
    for (int i = 0; i < nr_inputs; i++)
       create_node(NULL, "", 0);

    for (int j = 0; j <= nr_lines - 1; j++) {
        int node_inputs[circuit_nodes], nr_inputs = 0;
        char *token = strtok (input[j], " ");
        char *logical_operation = calloc(strlen(token) + 1, sizeof(char));
        strcpy(logical_operation, token);

        while (token != NULL) {
            char *token = strtok (NULL, " ");
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

//  citeste fisierul de intrare linie cu linie si salveaza tot intr-o matrice
char **read_input_file(FILE *input_file, int *nr_lines)
{
    char **input = NULL;
    char *line_to_read = NULL;
    size_t len_line = 0;

    ssize_t read = getline(&line_to_read, &len_line, input_file);
    while (read != -1) {
        //  line_to_read -- sir de elemente de tip char*
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

//  deschide fisierul de intrare, citeste linie cu linie si construieste circuitul
void parse_input(char *input)
{
    FILE *input_file = fopen(input, "r");
    fscanf (input_file, "%d %d\n", &nr_inputs, &output_node_number);
    
    //  se citeste pe linii (array de linii)
    int nr_lines = 0;
    char **data = read_input_file(input_file, &nr_lines);
    process_input(nr_lines, data, nr_inputs);
    for (int index = 0; index < nr_lines; index++)
        free(data[index]);
    free(data);
    fclose(input_file);
}

//  modifica radacina actuala a arborelui cu o alta radacina
void change_output_node_number(int new_output_node_number) 
{
    output_node_number = new_output_node_number;
}

//  legile 2 si 3 de Morgan
int split_complex_gate(int node_number, int parent_node_number, char *logical_operation)
{
    if (parent_node_number > 0)
        remove_input_from_node(node_number, parent_node_number);

    //  creez o poarta and daca am legea cu or sau or daca legea este cu and
    Node *new_node = create_node(NULL, logical_operation, 0);
    //  noua poarta construita va fi adaugata ca input pentru outputul portii not
    if (parent_node_number > 0)
        add_input_to_node(new_node->node_number, parent_node_number);
    
    //  pentru legile 2 si 3 construiesc o poarta NOT pentru fiecare input pe care o adaug in circuit:
    //  unei porti not ii adaug inputul curent
    //  poarta not o leg la poarta and/ or creata anterior 
    Node *input = get_input_node_of(node_number, 0);
    for (int index = 0; index < input->nr_inputs; index++) {
        Node *not_gate = create_node(NULL, "NOT", 0);
        add_input_to_node(not_gate->node_number, new_node->node_number);
        add_input_to_node(input->inputs[index], not_gate->node_number);
    }

    //  nemodificarea radacinii(output-ului) circuitului
    if (parent_node_number > 0)
        return 2;

    //  modificarea radacinii(outputu-ului) circuitului
    change_output_node_number(new_node->node_number);
    return 1;
}

//  aplic legilde de Morgan intregului circuit
int apply_de_morgan_laws(int node_number, int parent_node_number)
{
    Node *input = get_input_node_of(node_number, 0);
    if (strcmp(input->logical_operation, "") == 0)  //  frunza a circuitului
        return 0;
    
    //  legea 1 -> 2 negatii se anuleaza una pe alta
    if (strcmp(input->logical_operation, "NOT") == 0) {
        Node *input_not_gate = get_input_node_of(input->node_number, 0);
        if (parent_node_number > 0) {
            add_input_to_node(input_not_gate->node_number, parent_node_number);
            remove_input_from_node(node_number, parent_node_number);
            return 2;  //  radacina nu a fost modificata
        }
        //  in cazul in care radacina circuitului (output-ul) este o poarta not si apoi avem tot un not => 
        //  trebuie modificata radacina arborelui cu primul input-ul celui de-al doilea not
        change_output_node_number(input_not_gate->node_number);
        return 1;  //  radacina a fost modificata
    }

    //  legea 2: ! (a && b) => !a || !b
    if (strcmp(input->logical_operation, "AND") == 0)
        return split_complex_gate(node_number, parent_node_number, "OR");  

    //  legea3: !(a || b) => !a && !b
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

//  chemarea legilor de Morgan si stergerea portilor redundante
int simplify_circuit(int node_number, int parent_node_number)
{
    //  frunza => stop recursivitate
    if (strcmp(nodes[node_number]->logical_operation, "") == 0)
        return 0;

    //  legile de morgan pentru radacina
    int return_value = 0;
    if (strcmp (nodes[node_number]->logical_operation, "NOT") == 0) {
        int result = apply_de_morgan_laws(node_number, parent_node_number);
        if (result == 1) {
            // radacina a fost modificata => se mai aplica o data recursivitatea 
            simplify_circuit(output_node_number, 0);
            return 0;
        }
        if (result == 2)
            return_value = 1;
    }

    //   recursivitatea se continua prin apelarea fct pentru input-urile nodului curent
    int index = 0;
    while (index < nodes[node_number]->nr_inputs) {
        int inputs_modified = simplify_circuit(nodes[node_number]->inputs[index], node_number);
        /* doar daca nu am modificat o poarta inaintez, deoarece altfel am o poarta pe 
         * care nu am analizat-o inca si peste care as putea sari
         */
        if (!inputs_modified)
            index++;
    }

    //  eliminarea portilor redundante
    remove_unnecessary_nodes(node_number);
    return return_value;
}

/*  not_gate transf Tseitin => 
 * !b => (!a || !b) && (a || b), a = nr poartii not
 */
void transform_not_gate(int node_number)
{
    //  crearea unei porti not pentru node_number
    Node *not_gate_node = create_node(NULL, "NOT", 0);
    add_input_to_node(node_number, not_gate_node->node_number);
    
    //  crearea unei porti not pentru inputul portii not
    Node *not_gate_input_node = create_node(NULL, "NOT", 0);
    add_input_to_node(nodes[node_number]->inputs[0], not_gate_input_node->node_number);

    /*  (!a || !b)
     *  crearea portii or
     * adaugarea celor doua not-uri ca input-uri pentru aceasta
     */
    Node *or_gate_negations = create_node(NULL, "OR", 0);
    add_input_to_node(not_gate_node->node_number, or_gate_negations->node_number);
    add_input_to_node(not_gate_input_node->node_number, or_gate_negations->node_number);

    /*  (a || b)
     *  crearea portii or
     * adaugarea celor doua not-uri ca input-uri pentru aceasta
     */
    Node *or_gate = create_node(NULL, "OR", 0);
    add_input_to_node(node_number, or_gate->node_number);
    add_input_to_node(nodes[node_number]->inputs[0], or_gate->node_number);

    //  adaugarea celor doua ori-uri la radacina circuitului Tseitin (and)
    add_input_to_node(or_gate->node_number, tseitin_root_number);
    add_input_to_node(or_gate_negations->node_number, tseitin_root_number);

    //  modific node_number si inputul s-au a.i. sa fie frunze in circuit
    strcpy(nodes[node_number]->logical_operation, "");
    strcpy(get_input_node_of(node_number, 0)->logical_operation, "");
}

/*  and gate - Tseitin
 * (b && c) = (!a || b) && (!a || c) && (a || !b || !c), a = nr portii and
 */
void transform_and_gate(int node_number)
{
    //  or pentru ultima paranteza
    Node *or_gate = create_node(NULL, "OR", 0);

    //  adauga poarta or la radacina circuitului
    add_input_to_node(or_gate->node_number, tseitin_root_number);

    //  adaug nodul curent (a)
    add_input_to_node(node_number, or_gate->node_number);
    
    //  pentru fiecare input a lui a creez o poarta noua NOT ce devine input pentru poarta a
    for (int index = 0; index < nodes[node_number]->nr_inputs; index++) {
        Node *not_gate = create_node(NULL, "NOT", 0);
        add_input_to_node(not_gate->node_number, or_gate->node_number);
        add_input_to_node(nodes[node_number]->inputs[index], not_gate->node_number);
    }

    //  primele paranteze
    for (int index = 0; index < nodes[node_number]->nr_inputs; index++) {
        //  creez !a
        Node *not_gate_input = create_node(NULL, "NOT", 0);
        add_input_to_node(node_number, not_gate_input->node_number);

        //  adaug inputurile b si c ca inputuri ale portii or
        Node *or_gate_input = create_node(NULL, "OR", 0);
        add_input_to_node(not_gate_input->node_number, or_gate_input->node_number);
        add_input_to_node(nodes[node_number]->inputs[index], or_gate_input->node_number);

        //   adaug fiecare clauza nou formata la radacina circuitului Tseitin
        add_input_to_node(or_gate_input->node_number, tseitin_root_number);

        //  nodurile prelucrate devin frunze
        strcpy(get_input_node_of(node_number, index)->logical_operation, "");
    }
    //  nodul curent devine frunza
    strcpy(nodes[node_number]->logical_operation, "");
}

/*  Analog cu cazul de mai sus, dar aici avem:
 * (b || c) => (a || !b) && (a || !c) && (!a || b || c), unde a = nr portii or
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

//  apelarea unei fct de mai sus in fct de tipul portii nodului curent
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

//  fct recursiva pentru transformarea circuitului intr-un nou circuit
void recursive_tseitin(int node_number)
{
    //  e frunza
    if (strcmp(nodes[node_number]->logical_operation, "") == 0)
        return;

    //  parcurgere de la frunze la radacina  
    for (int index = 0; index < nodes[node_number]->nr_inputs; index++)
        recursive_tseitin(nodes[node_number]->inputs[index]);
    
    //  apelare transformare pentru nod curent
    transform_node(node_number);
}

//  transformarea Tseitin
int tseitin_transform(int node_number)
{
    //  retin numarul initial de noduri pentru afisarea in fisier
    tseitin_nr_inputs = circuit_nodes;

    //  creez noua radacina - AND
    Node *tseitin_root = create_node(NULL, "AND", 0);
    tseitin_root_number = tseitin_root->node_number;

    //  prelucrare radicina
    recursive_tseitin(node_number);

    //  adaug noua clauza formata la radacina and si nodul curent devine frunza in circuit
    add_input_to_node(node_number, tseitin_root_number);
    strcpy(nodes[node_number]->logical_operation, "");
}

/*  afisarea in fisierul de output conform cerintei
 */
void dimacs(char *output_file)
{
    FILE *output = fopen(output_file, "w+");
    fprintf(output, "p cnf %d %d\n", tseitin_nr_inputs, nodes[tseitin_root_number]->nr_inputs);
    //  parcurgerea input-urilor
    for (int index = 0; index < nodes[tseitin_root_number]->nr_inputs; index++) {
        Node *input_node = get_input_node_of(tseitin_root_number, index);
        //  daca avem o frunza => afisare directa
        if (strcmp(input_node->logical_operation, "") == 0) {
            fprintf(output, "%d 0\n", input_node->node_number);
            continue;
        }

        //  altfel, avem o poarta or => parcurgerea tuturor input-urilor
        int current_node = nodes[tseitin_root_number]->inputs[index]; 
        for (int index1 = 0; index1 < nodes[current_node]->nr_inputs; index1++) {
            Node *input_current_node = get_input_node_of(current_node, index1);
            //  e frunza => afisare
            if (strcmp(input_current_node->logical_operation, "") == 0)
                fprintf(output, "%d ", input_current_node->node_number);
            //  input de tip not => afisarea numarului nodului cu minus in fata
            else
                fprintf(output, "-%d ", input_current_node->inputs[0]);
        }
        fprintf(output, "0\n");
    }

    fclose(output);
}

//  dealocarea memoriei alocate
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
    //  prelucrare date de intrare
    parse_input(args[1]);

    //  simplificarea circuitului prin aplicarea legilor de Morgan si stergerea portilor redundante
    simplify_circuit(output_node_number, 0);

    //  aplicarea transformarii Tseitin
    tseitin_transform(output_node_number);

    //  afisarea in fisier
    dimacs(args[2]);

    //  dezalocarea memoriei alocate
    deallocate_memory();
    return 0;
}