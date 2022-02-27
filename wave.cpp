#define _USE_MATH_DEFINES

#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <cmath>
#include <mpi.h>

using namespace std;

//Note that this is a very simple serial implementation with a fixed grid and Neumann boundaries at the edges
//I am also using a vector of vectors, which is less efficient than allocating contiguous data.
vector<vector<double> > grid, new_grid, old_grid;
int imax = 500, jmax = 500;
double t_max = 30.0;
double t, t_out = 0.0, dt_out = 0.04, dt;
double y_max = 10.0, x_max = 10.0, dx, dy;
double c = 1;

int id, p;


int process_row, process_col;
MPI_Datatype Datatype_left, Datatype_right, Datatype_top, Datatype_bottom;
//this is the datatye used to send

vector<double> recv_left,recv_right,recv_top,recv_bottom;
//this is the buffer to store data from each side


/*********this is the function to create datatype, based on two dimension matrix*********/
void createdatatypes(double** data, int m, int n)
{
	vector<int> block_lengths;
	vector<MPI_Datatype> typelist;
	vector<MPI_Aint> addresses;
	MPI_Aint add_start;

	//left
	for (int i = 0; i < m; i++)
	{
		block_lengths.push_back(1);
		typelist.push_back(MPI_DOUBLE);
		MPI_Aint temp_address;
		MPI_Get_address(&data[i][0], &temp_address);
		addresses.push_back(temp_address);
	}
	MPI_Get_address(data, &add_start);
	for (int i = 0; i < m; i++) addresses[i] = addresses[i] - add_start;
	MPI_Type_create_struct(m, block_lengths.data(), addresses.data(), typelist.data(), &Datatype_left);
	MPI_Type_commit(&Datatype_left);

	//right
	block_lengths.resize(0);
	typelist.resize(0);
	addresses.resize(0);
	for (int i = 0; i < m; i++)
	{
		block_lengths.push_back(1);
		typelist.push_back(MPI_DOUBLE);
		MPI_Aint temp_address;
		MPI_Get_address(&data[i][n - 1], &temp_address);
		addresses.push_back(temp_address);
	}
	for (int i = 0; i < m; i++) addresses[i] = addresses[i] - add_start;
	MPI_Type_create_struct(m, block_lengths.data(), addresses.data(), typelist.data(), &Datatype_right);
	MPI_Type_commit(&Datatype_right);

	//top - only need one value
	int block_length = n;
	MPI_Datatype typeval = MPI_DOUBLE;
	MPI_Aint address;
	MPI_Get_address(data[0], &address);
	address = address - add_start;
	MPI_Type_create_struct(1, &block_length, &address, &typeval, &Datatype_top);
	MPI_Type_commit(&Datatype_top);

	//bottom - only need one value
	MPI_Get_address(data[m - 1], &address);
	address = address - add_start;
	MPI_Type_create_struct(1, &block_length, &address, &typeval, &Datatype_bottom);
	MPI_Type_commit(&Datatype_bottom);
}

void grid_to_file(int out)
{
	//Write the output for a single time step to file
	stringstream fname;
	fstream f1;
	fname << "./out/output"<<id << "_" << out << ".dat";
	f1.open(fname.str().c_str(), ios_base::out);
	for (int i = 0; i < imax; i++)
	{
		for (int j = 0; j < jmax; j++)
			f1 << grid[i][j] << "\t";
		f1 << endl;
	}
	f1.close();
}

//Do a single time step
void do_iteration(void)
{
	//Calculate the new displacement for all the points not on the boundary of the domain
	//Note that in parallel the edge of processor's region is not necessarily the edge of the domain
	for (int i = 1; i < imax -1; i++){
		for (int j = 1; j < jmax -1; j++){
			new_grid[i][j] = pow(dt * c, 2.0) * ((grid[i + 1][j] - 2.0 * grid[i][j] + grid[i - 1][j]) / pow(dx, 2.0)
			+ (grid[i][j + 1] - 2.0 * grid[i][j] + grid[i][j - 1]) / pow(dy, 2.0)) + 2.0 * grid[i][j] - old_grid[i][j];
		}
	}
	
	//Implement boundary conditions some changes to achieve this function, we have to justify which process could calculate the boundry
	for (int i = 1; i < imax-1; i++)
	{
		if(id%process_col == 0){
			new_grid[i][0] = new_grid[i][1];
		}else{
			new_grid[i][0] = pow(dt * c, 2.0) * ((grid[i + 1][0] - 2.0 * grid[i][0] + grid[i - 1][0]) / pow(dx, 2.0)
			+ (grid[i][0 + 1] - 2.0 * grid[i][0] + recv_left[i]) / pow(dy, 2.0)) + 2.0 * grid[i][0] - old_grid[i][0];
		}
		if(id%process_col == process_row-1){
			new_grid[i][jmax - 1] = new_grid[i][jmax - 2];
		}else{
			new_grid[i][jmax - 1] = pow(dt * c, 2.0) * ((grid[i + 1][jmax - 1] - 2.0 * grid[i][jmax - 1] + grid[i - 1][jmax - 1]) / pow(dx, 2.0)
			+ (recv_right[i] - 2.0 * grid[i][jmax - 1] + grid[i][jmax - 2]) / pow(dy, 2.0)) + 2.0 * grid[i][jmax - 1] - old_grid[i][jmax - 1];
		}
	}

	//Implement boundary conditions some changes to achieve this function, we have to justify which process could calculate the boundry
	for (int j = 1; j < jmax-1; j++)
	{
		if(id/process_col == 0){
			new_grid[0][j] = new_grid[1][j];
		}else{	
			new_grid[0][j] = pow(dt * c, 2.0) * ((grid[0 + 1][j] - 2.0 * grid[0][j] + recv_top[j]) / pow(dx, 2.0)
			+ (grid[0][j + 1] - 2.0 * grid[0][j] + grid[0][j - 1]) / pow(dy, 2.0)) + 2.0 * grid[0][j] - old_grid[0][j];
		}
		if(id/process_col == process_col-1){
			new_grid[imax-1][j] = new_grid[imax-2][j];
		}else{
			new_grid[imax-1][j] = pow(dt * c, 2.0) * ((recv_bottom[j] - 2.0 * grid[imax-1][j] + grid[imax - 2][j]) / pow(dx, 2.0)
			+ (grid[imax-1][j + 1] - 2.0 * grid[imax-1][j] + grid[imax-1][j - 1]) / pow(dy, 2.0)) + 2.0 * grid[imax-1][j] - old_grid[imax-1][j];
		}
		
	}

	t += dt;

	//Note that I am not copying data between the grids, which would be very slow, but rather just swapping pointers
	old_grid.swap(new_grid);
	old_grid.swap(grid);
}

int main(int argc, char *argv[])
{
	

	MPI_Init(&argc, &argv);

	MPI_Comm_rank(MPI_COMM_WORLD, &id);
	MPI_Comm_size(MPI_COMM_WORLD, &p);
    MPI_Request* request = new MPI_Request[8];
    int tag_num = 1;

	process_row = static_cast<int>(sqrt(p));
	process_col = p/process_row;
	int row_allocate, col_allocate;

	//this is the process to divide each part to trangle part, the last one whill take the largest part
	if(id%process_col == process_col - 1){
		row_allocate = imax - static_cast<int>(imax/process_row) * (process_row-1);
		col_allocate = jmax - static_cast<int>(jmax/process_col) * (process_col-1);
	}else{
		row_allocate = imax/process_row;
		col_allocate = jmax/process_col;
	}


	imax = row_allocate;
	jmax = col_allocate;

	old_grid.resize(imax, vector<double>(jmax));
	grid.resize(imax, vector<double>(jmax));
	new_grid.resize(imax, vector<double>(jmax));

	dx = x_max / ((double)imax - 1);
	dy = y_max / ((double)imax - 1);

	t = 0.0;

	dt = 0.1 * min(dx, dy) / c;
	int out_cnt = 0, it = 0;

	//sets half sinusoidal intitial disturbance - this is a bit brute force and it can be done more elegantly
	if(id == 0){
	double r_splash = 1.0;
	double x_splash = 3.0;
	double y_splash = 3.0;
	for (int i = 1; i < imax - 1; i++)
		for (int j = 1; j < jmax - 1; j++)
		{
			double x = dx * i;
			double y = dy * j;

			double dist = sqrt(pow(x - x_splash, 2.0) + pow(y - y_splash, 2.0));

			if (dist < r_splash)
			{
				double h = 5.0*(cos(dist / r_splash * M_PI) + 1.0);

				grid[i][j] = h;
				old_grid[i][j] = h;
			}
		}
	}

	grid_to_file(out_cnt);
	out_cnt++;
	t_out += dt_out;

	double **data_array;
	data_array = new double*[imax];
	for (int i = 0; i < imax; i++)
		data_array[i] = new double[jmax];

	//initialize the matrix
	recv_bottom.resize(jmax);
	recv_left.resize(imax);
	recv_top.resize(jmax);
	recv_right.resize(imax);
				
	createdatatypes(data_array,imax,jmax);

	while (t < t_max)
	{
		int count = 0;
		do_iteration();
		for(int i = 0 ; i < imax; ++i){

			if(id%process_col != 0){
				data_array[i][0] = grid[i][0];
			}
			//except left

			if(id%process_col != process_col-1){
				data_array[i][jmax-1] = grid[i][jmax-1];
			}
		}
		for(int i = 0 ; i < jmax; ++i){
			if(id/process_col != 0){
				data_array[0][i] = grid[0][i];
			}
			//except top

			if(id/process_col != process_row-1){
				data_array[imax-1][i] = grid[imax-1][i];
			}
		}

		//based on process number, ditermine which side to send
		if(id/process_col != 0){
			MPI_Isend(data_array, 1, Datatype_top, id-process_col, tag_num, MPI_COMM_WORLD, &request[count]);
			++count;

			MPI_Irecv(recv_top.data(), jmax, MPI_DOUBLE,id-process_col, tag_num, MPI_COMM_WORLD, &request[count]);
			++count;
		}
		//except top

		//based on process number, ditermine which side to send
		if(id%process_col != 0){

			MPI_Isend(data_array, 1, Datatype_left,id-1, tag_num, MPI_COMM_WORLD, &request[count]);
			++count;

			MPI_Irecv(recv_left.data(), imax, MPI_DOUBLE,id-1, tag_num, MPI_COMM_WORLD, &request[count]);
			++count;
			
		}
		//except left

		//based on process number, ditermine which side to send
		if(id/process_col != process_row-1){
			MPI_Isend(data_array, 1, Datatype_bottom,id+process_col, tag_num, MPI_COMM_WORLD, &request[count]);
			++count;

			MPI_Irecv(recv_bottom.data(), jmax, MPI_DOUBLE,id+process_col, tag_num, MPI_COMM_WORLD, &request[count]);
			++count;
			
		}
		//except bottom

		//based on process number, ditermine which side to send
		if(id%process_col != process_col-1){
			MPI_Isend(data_array, 1, Datatype_right,id+1, tag_num, MPI_COMM_WORLD, &request[count]);
			++count;

			MPI_Irecv(recv_right.data(), imax, MPI_DOUBLE,id+1, tag_num, MPI_COMM_WORLD, &request[count]);
			++count;
			
		}
		//except right

				

		MPI_Waitall(count, request, MPI_STATUS_IGNORE);

		//Note that I am outputing at a fixed time interval rather than after a fixed number of time steps.
		//This means that the output time interval will be independent of the time step (and thus the resolution)
		if (t_out <= t)
		{
			cout << "output: " << out_cnt << "\tt: " << t << "\titeration: " << it << endl;
			grid_to_file(out_cnt);
			out_cnt++;
			t_out += dt_out;
		}

		it++;

        MPI_Barrier(MPI_COMM_WORLD);
	}
	MPI_Type_free(&Datatype_left);
	MPI_Type_free(&Datatype_right);
	MPI_Type_free(&Datatype_top);
	MPI_Type_free(&Datatype_bottom);

	MPI_Finalize();
	return 0;
}