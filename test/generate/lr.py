
import struct
import factor_graph_pb2
import random

NVAR = 1000000

fo = open("lr_learn/graph.meta.pb", "wb")
w = factor_graph_pb2.FactorGraph()
w.numWeights = 1
w.numVariables = NVAR*2
w.numFactors = NVAR*2
w.numEdges = NVAR*2
w.weightsFile = ""
w.variablesFile = ""
w.factorsFile = ""
w.edgesFile = ""
fo.write(w.SerializeToString())
fo.close()

fo = open("lr_learn/graph.variables.pb", "wb")
for i in range(0,NVAR):
	v = factor_graph_pb2.Variable()
	v.id = i
	if random.random() < 0.8:
		v.initialValue = 1
	else:
		v.initialValue = 0
	v.dataType = 0
	v.isEvidence = True
	v.cardinality = 1
	v.edgeCount = 1
	size = v.ByteSize()
	fo.write(struct.pack("i", size + 3))
	fo.write(v.SerializeToString())

for i in range(NVAR,2*NVAR):
	v = factor_graph_pb2.Variable()
	v.id = i
	v.initialValue = 0
	v.dataType = 0
	v.isEvidence = False
	v.cardinality = 1
	v.edgeCount = 1
	size = v.ByteSize()
	fo.write(struct.pack("i", size + 3))
	fo.write(v.SerializeToString())
fo.close()

fo = open("lr_learn/graph.factors.pb", "wb")
for i in range(0,NVAR):
	f = factor_graph_pb2.Factor()
	f.id = i
	f.weightId = 0
	f.factorFunction = 0
	f.edgeCount = 1
	fo.write(struct.pack("i", f.ByteSize()+3))
	fo.write(f.SerializeToString())
for i in range(NVAR,2*NVAR):
	f = factor_graph_pb2.Factor()
	f.id = i
	f.weightId = 0
	f.factorFunction = 0
	f.edgeCount = 1
	fo.write(struct.pack("i", f.ByteSize()+3))
	fo.write(f.SerializeToString())
fo.close()

fo = open("lr_learn/graph.edges.pb", "wb")
for i in range(0,NVAR):
	e = factor_graph_pb2.GraphEdge()
	e.variableId = i
	e.factorId = i
	e.position = 0
	e.isPositive = True
	fo.write(struct.pack("i", e.ByteSize()+3))
	fo.write(e.SerializeToString())
for i in range(NVAR,2*NVAR):
	e = factor_graph_pb2.GraphEdge()
	e.variableId = i
	e.factorId = i
	e.position = 0
	e.isPositive = True
	fo.write(struct.pack("i", e.ByteSize()+3))
	fo.write(e.SerializeToString())
fo.close()

fo = open("lr_learn/graph.weights.pb", "wb")
w = factor_graph_pb2.Weight()
w.id = 0
w.initialValue = 0.0
w.isFixed = False
fo.write(struct.pack("i", w.ByteSize()+3))
fo.write(w.SerializeToString())
fo.close()



