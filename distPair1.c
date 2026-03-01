/*
 Distributed Pairing (Client-Server) using MPI

 Algorithm (short explanation):
 - Process with rank 0 acts as the teacher (server).
 - All other processes act as students (clients).
 - Each student sends a pairing request (its rank) to the teacher.
 - The teacher receives requests in arrival order and pairs students
   sequentially.
 - If there is an odd number of students, the last one is paired with itself.
 - The teacher sends the partner rank back to each student.
 - Each student prints its own rank and its partner.
*/

#include <mpi.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    int rank, size;

    MPI_Init(&argc, &argv);              // Start MPI environment
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); // Get process ID
    MPI_Comm_size(MPI_COMM_WORLD, &size); // Total number of processes

    int partner;

    if (rank == 0) {
        /* ===== Teacher (Server) ===== */

        int num_students = size - 1;     // Exclude teacher
        int requests[num_students];
        MPI_Status status;

        // Receive pairing requests from all students
        for (int i = 0; i < num_students; i++) {
            MPI_Recv(&requests[i], 1, MPI_INT, MPI_ANY_SOURCE,
                     0, MPI_COMM_WORLD, &status);
        }

        // Pair students sequentially
        for (int i = 0; i < num_students; i += 2) {
            int s1 = requests[i];
            int s2;

            if (i + 1 < num_students)
                s2 = requests[i + 1];
            else
                s2 = s1; // Odd case: partner with self

            // Send partner info back
            MPI_Send(&s2, 1, MPI_INT, s1, 1, MPI_COMM_WORLD);
            if (s1 != s2)
                MPI_Send(&s1, 1, MPI_INT, s2, 1, MPI_COMM_WORLD);
        }

    } else {
        /* ===== Student (Client) ===== */

        int my_id = rank;

        // Send pairing request to teacher
        MPI_Send(&my_id, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);

        // Receive partner from teacher
        MPI_Recv(&partner, 1, MPI_INT, 0, 1, MPI_COMM_WORLD,
                 MPI_STATUS_IGNORE);
        printf("Student %d partnered with %d\n", rank, partner);
    }

    MPI_Finalize(); // End MPI
    return 0;
}