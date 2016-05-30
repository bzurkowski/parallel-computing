all:
	# nvcc bandwidth_test.cu -o bandwidth_test -gencode arch=compute_20,code=sm_20 -I .
	# nvcc device_query.cu -o device_query -gencode arch=compute_20,code=sm_20 -I .
	# nvcc add_vectors.cu -o add_vectors -gencode arch=compute_20,code=sm_20 -I .
	nvcc matmul.cu -o matmul -I ./helpers -std=c++11 -lcublas

clean:
	# rm bandwidth_test
	# rm device_query
	rm matmul
