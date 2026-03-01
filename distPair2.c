
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define TAG_START 1
#define TAG_TOKEN 2
#define TAG_REQ   3
#define TAG_ACK   4
#define TAG_DONE  5

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int students = size - 1;   // exclude teacher
    srand(time(NULL) + rank);

    if (rank == 0) {
        // -------- Teacher --------
        int start = (rand() % students) + 1;
        int token[1] = { students }; // remaining unpaired

        MPI_Send(token, 1, MPI_INT, start, TAG_TOKEN, MPI_COMM_WORLD);

        // wait for completion
        MPI_Recv(token, 1, MPI_INT, MPI_ANY_SOURCE, TAG_DONE,
                 MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // broadcast done to everyone
        for (int i = 1; i <= students; i++)
            MPI_Send(NULL, 0, MPI_INT, i, TAG_DONE, MPI_COMM_WORLD);
    }
    else {
        // -------- Student --------
        int partner = -1;
        int paired = 0;
        MPI_Status status;

        while (1) {
            // probe to see what message arrives
            MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

            if (status.MPI_TAG == TAG_TOKEN) {
                int token;
                MPI_Recv(&token, 1, MPI_INT, status.MPI_SOURCE,
                         TAG_TOKEN, MPI_COMM_WORLD, &status);

                if (!paired) {
                    int target;
                    do {
                        target = (rand() % students) + 1;
                    } while (target == rank);

                    MPI_Send(&rank, 1, MPI_INT, target, TAG_REQ, MPI_COMM_WORLD);

                    int response;
                    MPI_Recv(&response, 1, MPI_INT, target, TAG_ACK,
                             MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                    if (response == 1) {
                        partner = target;
                        paired = 1;
                        token -= 2;
                    }
                }

                if (token <= 0) {
                    MPI_Send(&token, 1, MPI_INT, 0, TAG_DONE, MPI_COMM_WORLD);
                } else {
                    int next = (rank % students) + 1;
                    MPI_Send(&token, 1, MPI_INT, next, TAG_TOKEN, MPI_COMM_WORLD);
                }
            }
            else if (status.MPI_TAG == TAG_REQ) {
                int sender;
                MPI_Recv(&sender, 1, MPI_INT, status.MPI_SOURCE,
                         TAG_REQ, MPI_COMM_WORLD, &status);

                int ok = 0;
                if (!paired) {
                    partner = sender;
                    paired = 1;
                    ok = 1;
                }
                MPI_Send(&ok, 1, MPI_INT, sender, TAG_ACK, MPI_COMM_WORLD);
            }
            else if (status.MPI_TAG == TAG_DONE) {
                MPI_Recv(NULL, 0, MPI_INT, status.MPI_SOURCE,
                         TAG_DONE, MPI_COMM_WORLD, &status);
                break;
            }
        }

        printf("Student %d paired with %d\n", rank, partner);
    }

    MPI_Finalize();
    return 0;
}