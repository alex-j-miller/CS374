def generate_slurm_script(program_name, max_nodes, max_processes):
    for nodes in range(1, max_nodes + 1):
        for processes in range(1, max_processes + 1):
            
            if ((nodes in [1,2,4,8,16] and processes == 1) or (nodes == 16 and processes in [1,2,4,8,16])):
            
                with open(f'script_{program_name}_{nodes}_{processes}.slurm', 'w') as file:
                    file.write(f'''#!/bin/bash
#
# Set the number of nodes to use (max 20)
#SBATCH -N {nodes}
#
# Set the number of processes per node (max 16)
#SBATCH --ntasks-per-node={processes}
#

# Load the compiler and MPI library
module load openmpi-2.0/gcc

# Run the program
mpirun ./{program_name}
''')

program_name = input("Enter the program name: ")
max_nodes = 16  # You can change this to the desired max nodes
max_processes = 16  # You can change this to the desired max processes

generate_slurm_script(program_name, max_nodes, max_processes)

