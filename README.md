Each node is kept in a empty array of nodes, where at its index is node i from the circuit.

Each node is, in fact, a structure in which are retained:
- the logical operation specific to the node ("AND", "OR", "NOT or "" - leaf node)
- the number of inputs for the node
- what are its inputs (their identifiers)
- what is the node identifier (node_number)

----
- circuit_nodes = the number of nodes in the circuit - it increases with each new node created, initially being 0
- nr_inputs = number of inputs in the circuit (leaves)
- output_node_number = the root of the circuit read from the file

I used the Tseitin discount which I will explain below. Variables used:
- tseitin_root_number = the root of a new tree built by transforming Tseitin based on the initial one
- tseitin_nr_inputs = the number of inputs of the new tree

The nodes in the circuit are kept in the form of a tree.
In the implementation of this theme, I worked more with the node identifiers (node_number) in the circuit, than with the actual nodes.

Steps implemented:
--
     1. traversing the input file, reading and processing the data, building the circuit
    
     2. Simplifying the circuit by applying Morgan's laws and deleting redundant gates.
         Morgan's Laws:

             1. two NOTs cancel each other. We know that a NOT is guaranteed to have only one input => this input will not be modified.
                 Because the NOTs cancel => we can get rid of these two gates, taking the input of the second NOT
                 as input for the gate that had as input the first NOT. Here there is the possibility that the first NOT is the output
                 the circuit, in which case the root of the circuit will change.

             2. !(a || b) = !a && !b
                 a new AND gate is created
                 for each input of the OR gate, a NOT gate is created that has as input, the input of the OR gate
                 the two NOTs created become inputs for the AND gate created
                 the OR gate is disconnected from the circuit, and the created AND gate appears in its place

             3. !(a && b) = !a || b
                 a new OR gate is created
                 for each input of the AND gate, a NOT gate is created that has as input the input of the AND gate
                 the two created NOTs become inputs for the created OR gate
                 the AND gate is disconnected from the circuit, and the created OR gate appears in its place

             Morgan's laws are applied recursively throughout the circuit.

         Deleting redundant ports, a process also carried out recursively:
             x1 && (x2 && x3 && (x4 && x5)) = x1 && (x2 && x3 && x4 && x5) = x1 && x2 && x3 && x4 && x5
             x1 || (x2 || x3 || (x4 || x5)) = x1 || (x2 || x3 || x4 || x5) = x1 || x2 || x3 || x4 || x5

     3. Distributivity is achieved with the help of the Tseitin transformation
         -- description link: https://profs.info.uaic.ro/~stefan.ciobaca/logic-2018-2019/notes7.pdf) --
         It has been demonstrated that if I reach an equisatisfactory circuit, the circuit from which I started is also satisfactory.

         I create an AND gate which is the new root of the circuit.
         the circuit from the leaves to the root is processed

         1. !b => (!a || !b) && (a || b), a = NOT gate number
             I create two NOT gates like this:
             one is the input of a, and the other is the input of b => it is added to the inputs of the two nodes
             I create an OR gate that is input for the created root
             to the or gate I add as input the two previously created NOT gates

             I create an OR gate that has a and b as inputs
             this gate becomes the input for the root and created previously

         2. (b && c) = (!a || b) && (!a || c) && (a || !b || !c), a = number of gates and
             for the last bracket:
             a gate is created or
             for each input of the a gate, a NOT gate is created which becomes the input for the or gate
             and each input becomes input for the created NOT gate
             this OR gate becomes the input for the root of the newly created tree

             for the first two brackets:
             a NOT gate is created that has an AND gate as input
             an OR gate is created that has as inputs the NOT gate and the current input
             the OR gate becomes the input for the root of the tree

         3. (b || c) => (a || !b) && (a || !c) && (!a || b || c), where a = number of OR gates
             for the last bracket:
             an OR gate is created
             for gate a, a NOT gate is created whose input is gate a
             the newly created NOT gate and the inputs of gate a (b and c) become inputs for the created OR gate
             this OR gate becomes the input for the root of the newly created tree

             for the first two brackets:
             for each input of gates a, a NOT gate is created that has as input the current input of gate a
             an OR gate is created that has as inputs the NOT gate of the current input and the current node
             the OR gate becomes the input for the root of the tree

     4. Display in the file:
         After the Tseitin transformation: we will have the AND root with leaves or OR gates as inputs, if we meet:
             - leaves, they are displayed directly in the file
             - if we encounter OR gates => we must go through their neighbors which are either NOT gates or leaves
             - the leaves are displayed directly, and the NOT gate input is displayed with a minus in front
