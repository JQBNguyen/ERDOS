# ERDOS
ERDOS (Eulerian Routing of DNA Origami Scaffolds) is a program used for the automated design of finding A-trail scaffold routings for toroidal DNA polyhedra.

## Usage
The program takes a PLY file and a shape name (as a string) as command line arguments or user input and outputs text files representing the found A-trail(s). 

Execution with given arguments:
```
./ERDOS <input_PLY_file> <given_shape_name> <number_of_parallel_branches> <produce_checkpoint_files> <list_of_given_checkpoint_files>(optional)
```

Execution with incomplete number of arguments:
```
./ERDOS <insufficient_arguments...>
>> Please input file path name (provided file should be of PLY format).
<input_PLY_file_path>
>> Please input desired shape name for output files (any output files will use this as an identifier).
<given_shape_name>
>> Do you want to produce checkpoint files (periodically store covering tree search information to save progress and rerun ERDOS at later times)? (Y/N)
Y
>> Are you providing checkpoint files? (Y/N)
N
...
```

### Command Line Arguments Description
- <input_PLY_file>: PLY mesh filepath to find an A-trail scaffold routing for.
- <given_shape_name>: User provided shape name to be used in output file naming.
- <number_of_parallel_branches>: Maximum number of parallel branches ERDOS will search through when performing the covering tree search.
- <produce_checkpoint_files>: Whether or not user wants ERDOS to output checkpoint files to continue covering tree search(es) at later times.
- <list_of_given_checkpoint_files>(optional): Filepath(s) of checkpoint files for ERDOS to parse and read. Covering tree search(es) will continue based on these files.

## Output
ERDOS can output A-trail files and checkpoint files.

### A-trail Output
Outputs text files of the A-trail as a list of vertices, separated by whitepsace. For TXT file, vertices are indexed by 1, and for NTRAIL file, vertices are index by 0.

List of output attributes specified in file name:
- shape name
- branch number
- "color" of covering tree
- presence of crossing staples

Sample Output File Names:
```
atrail_tetrahedron_0_blue_no_crossing_staples.txt
atrail_tritorus_3_red_no_crossing_staples.ntrail
```

### Checkpoint Output
Outputs text files that store covering tree search information to be read at a later time to continue covering tree search. Lists current iteration's vertex stack separated by whitespace followed by '|' and the following information: current vertex, vertex inclusion choice, color of covering tree search, specified parallel branch number.

Sample Output File Names:
```
covering_tree_tetrahedron_blue_branch_0_checkpoint.txt
covering_tree_tritorus_red_branch_3_checkpoint.txt
```

## Compilation
Windows: Use the provided Makefile or CMake file.  

macOS/Linux: Use the provided CMake file.  

The files can also be compiled manually.

Parallelization Requirement: You must have OpenMP installed.

Note: ERDOS can function as a serial program without parallelization.

## Contributors
- Johnny Nguyen
- Matthew Vu
- Seth Gonzalez
- Abdulmelik Mohammed

Special thanks to ...
- Antii Elonen
- Ahmad Kaddoura
- Dung Phan
- Hao Legaspi
- Han Nguyen
- Sonja Durr
