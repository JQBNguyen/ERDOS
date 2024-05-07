all: ERDOS

ERDOS: ply_to_embedding.o make_cc.o edgecode.o cc_embedded_graph.o bb_covering_tree.o find_a_trail.o erdos.o
	g++ -o ERDOS ply_to_embedding.o make_cc.o edgecode.o cc_embedded_graph.o bb_covering_tree.o find_a_trail.o erdos.o

ply_to_embedding.o: ply_to_embedding.cpp ply_to_embedding.h
	g++ -c ply_to_embedding.cpp

make_cc.o: make_cc.cpp make_cc.h
	g++ -c make_cc.cpp

edgecode.o: edgecode.cpp edgecode.h
	g++ -c edgecode.cpp

cc_embedded_graph.o: cc_embedded_graph.cpp cc_embedded_graph.h
	g++ -c cc_embedded_graph.cpp

bb_covering_tree.o: bb_covering_tree.cpp bb_covering_tree.h
	g++ -c bb_covering_tree.cpp

find_a_trail.o: find_a_trail.cpp find_a_trail.h
	g++ -c find_a_trail.cpp