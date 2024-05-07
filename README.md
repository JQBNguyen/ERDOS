# ERDOS
ERDOS (Eulerian Routing of DNA Origami Scaffolds) is a program used for the automated design of finding A-trail scaffold routings for toroidal DNA polyhedra.

## Usage
The program takes a PLY file and a shape name (as a string) as command line arguments or user input. The output is a TXT file of the A-trail as a list of vertices, indexed at 1, separated by whitespace.

## Compilation
The provided Makefile can be used to compile the code into an executable named
```
ERDOS.exe
```
or can be manually compiled.

## Sample Execution and Output
If the program is run without command line arguments, using
```
./ERDOS
```
then the following output messages will be displayed to the user.
```
No file given. Please input file name.
<user input here>
No shape name given. Please input desired shape name.
<user input here>
```

The program can also be run with one command line argument, using
```
./ERDOS tetrahedron.ply
```
then the following output messages will be displayed to the user.
```
No shape name given. Please input desired shape name.
<user input here>
```

When the program is run with two command line arguments,
```
./ERDOS <ply file> <shape_name>
```
the user will not be further prompted for input.

The program will output the A-trail into a file named
```
atrail_<shape_name>_<color>_obj.txt
```

The sample execution using the provided PLY file,
```
./ERDOS tetrahedron.ply tetrahedron
```
results in the output TXT file,
```
atrail_tetrahedron_red_obj.txt
```
with the following information:
```
1 2 3 4 2 1 4 1 3 1
```

## Contributors
- Johnny Nguyen
- Matthew Vu
- Seth Gonzalez
- Abdulmelik Mohammed

Special thanks to ...
- Antii Elonen
- Dung Phan
- Hao Legaspi
- Han Nguyen
- Sonja Durr
