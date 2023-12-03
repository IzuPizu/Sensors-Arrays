#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "structs.h"
#define MAX_SIZE 100

void get_operations(void **operations);

// sort array of sensors using Insertion Sort (stable algorithm)
void sort_array(sensor *Sensor_array, int num_sensors) {
    int j, i;
    sensor aux;
    for (i = 1; i < num_sensors; i++) {
        j = i - 1;
        aux = Sensor_array[i];
        while (j >= 0 && (Sensor_array[j].sensor_type == TIRE &&
                          aux.sensor_type == PMU)) {
            Sensor_array[j + 1] = Sensor_array[j];
            j = j - 1;
        }
        Sensor_array[j + 1] = aux;
    }
}

// free the memory
void free_memory(sensor *Sensor_array, int num_sensors) {
    int i;
    for (i = 0; i < num_sensors; i++) {
        free(Sensor_array[i].sensor_data);
        free(Sensor_array[i].operations_idxs);
    }
    free(Sensor_array);
}

int main(int argc, char const *argv[]) {
    // open binary file for reading
    FILE *fptr;
    fptr = fopen(argv[1], "rb");

    if (argc != 2) {
        exit(1);
    }
    if (fptr == NULL) {
        exit(1);
    }

    // read number of sensors
    int num_sensors;
    fread(&num_sensors, sizeof(int), 1, fptr);

    // allocate space for num_sensors sensors in array
    sensor *Sensor_array;  // array of structs
    Sensor_array = malloc(num_sensors * sizeof(sensor));

    int i, j, type;
    // read data for each sensor and allocate space
    for (i = 0; i < num_sensors; i++) {
        fread(&type, sizeof(int), 1, fptr);

        if (type == 0) {
            Sensor_array[i].sensor_type = TIRE;
            // allocate memory for tire_sensor struct
            Sensor_array[i].sensor_data = malloc(sizeof(tire_sensor));
            tire_sensor *Tsensor = (tire_sensor *)Sensor_array[i].sensor_data;

            fread(&Tsensor->pressure, sizeof(float), 1, fptr);
            fread(&Tsensor->temperature, sizeof(float), 1, fptr);
            fread(&Tsensor->wear_level, sizeof(int), 1, fptr);
            fread(&Tsensor->performace_score, sizeof(int), 1, fptr);

            // read number of operations
            fread(&Sensor_array[i].nr_operations, sizeof(int), 1, fptr);
            // read array of operations
            Sensor_array[i].operations_idxs =
                malloc(sizeof(int) * Sensor_array[i].nr_operations);
            if (Sensor_array[i].operations_idxs == NULL) {
                exit(1);
            }
            fread(Sensor_array[i].operations_idxs, sizeof(int),
                  Sensor_array[i].nr_operations, fptr);

        } else if (type == 1) {
            Sensor_array[i].sensor_type = PMU;
            // allocate memory for power_management_unit struct
            Sensor_array[i].sensor_data = malloc(sizeof(power_management_unit));
            power_management_unit *Psensor =
                (power_management_unit *)Sensor_array[i].sensor_data;

            fread(&Psensor->voltage, sizeof(float), 1, fptr);
            fread(&Psensor->current, sizeof(float), 1, fptr);
            fread(&Psensor->power_consumption, sizeof(float), 1, fptr);
            fread(&Psensor->energy_regen, sizeof(int), 1, fptr);
            fread(&Psensor->energy_storage, sizeof(int), 1, fptr);

            // read number of operations
            fread(&Sensor_array[i].nr_operations, sizeof(int), 1, fptr);
            // read array of operations
            Sensor_array[i].operations_idxs =
                malloc(sizeof(int) * Sensor_array[i].nr_operations);
            if (Sensor_array[i].operations_idxs == NULL) {
                exit(1);
            }
            fread(Sensor_array[i].operations_idxs, sizeof(int),
                  Sensor_array[i].nr_operations, fptr);
        }
    }

    // sort array
    sort_array(Sensor_array, num_sensors);

    // get operations array
    void *operations_arr[8];
    get_operations(operations_arr);

    // read commands
    char buffer[MAX_SIZE];
    int index;

    scanf("%s", buffer);
    while (strcmp(buffer, "exit") != 0) {
        if (strcmp(buffer, "clear") == 0) {
            // delete sensors that contain wrong values
            for (i = num_sensors - 1; i >= 0; i--) {
                if (Sensor_array[i].sensor_type == TIRE) {
                    tire_sensor *Tsensor =
                        (tire_sensor *)Sensor_array[i].sensor_data;
                    int *ptr = (int *)Sensor_array[i].operations_idxs;
                    if (Tsensor->pressure < 19 || Tsensor->pressure > 28 ||
                        Tsensor->temperature < 0 ||
                        Tsensor->temperature > 120 || Tsensor->wear_level < 0 ||
                        Tsensor->wear_level > 100) {
                        // delete TIRE sensor from array
                        for (j = i; j < num_sensors - 1; j++) {
                            Sensor_array[j] = Sensor_array[j + 1];
                        }
                        num_sensors--;
                        Sensor_array =
                            realloc(Sensor_array, sizeof(sensor) * num_sensors);
                        free(Tsensor);
                        free(ptr);
                    }
                }
                if (Sensor_array[i].sensor_type == PMU) {
                    power_management_unit *Psensor =
                        (power_management_unit *)Sensor_array[i].sensor_data;
                    int *ptr = (int *)Sensor_array[i].operations_idxs;
                    if (Psensor->voltage < 10 || Psensor->voltage > 20 ||
                        Psensor->current < -100 || Psensor->current > 100 ||
                        Psensor->power_consumption < 0 ||
                        Psensor->power_consumption > 1000 ||
                        Psensor->energy_regen < 0 ||
                        Psensor->energy_regen > 100 ||
                        Psensor->energy_storage < 0 ||
                        Psensor->energy_storage > 100) {
                        // delete PMU sensor from array
                        for (j = i; j < num_sensors - 1; j++) {
                            Sensor_array[j] = Sensor_array[j + 1];
                        }
                        num_sensors--;
                        Sensor_array =
                            realloc(Sensor_array, sizeof(sensor) * num_sensors);
                        free(Psensor);
                        free(ptr);
                    }
                }
            }
        }
        // analyze sensors
        if (strcmp(buffer, "analyze") == 0) {
            scanf("%d", &index);
            if (index < 0 || index > num_sensors - 1) {
                printf("Index not in range!\n");

            } else {
                int idxs_value;
                // go through operation_idxs array and call function
                for (j = 0; j < Sensor_array[index].nr_operations; j++) {
                    idxs_value = Sensor_array[index].operations_idxs[j];
                    void (*op)(void *) = operations_arr[idxs_value];
                    op(Sensor_array[index].sensor_data);
                }
            }
        }
        // print array of sensors
        if (strcmp(buffer, "print") == 0) {
            scanf("%d", &index);
            if (index < 0 || index > num_sensors - 1) {
                printf("Index not in range!\n");

            } else {
                if (Sensor_array[index].sensor_type == TIRE) {
                    tire_sensor *Tsensor =
                        (tire_sensor *)Sensor_array[index].sensor_data;

                    printf("Tire Sensor\n");
                    printf("Pressure: %.2f\n", Tsensor->pressure);
                    printf("Temperature: %.2f\n", Tsensor->temperature);
                    printf("Wear Level: %d%%\n", Tsensor->wear_level);
                    if (Tsensor->performace_score != 0) {
                        printf("Performance Score: %d\n",
                               Tsensor->performace_score);
                    } else {
                        printf("Performance Score: Not Calculated\n");
                    }
                } else if (Sensor_array[index].sensor_type == PMU) {
                    power_management_unit *Psensor =
                        (power_management_unit *)Sensor_array[index]
                            .sensor_data;
                    printf("Power Management Unit\n");
                    printf("Voltage: %.2f\n", Psensor->voltage);
                    printf("Current: %.2f\n", Psensor->current);
                    printf("Power Consumption: %.2f\n",
                           Psensor->power_consumption);
                    printf("Energy Regen: %d%%\n", Psensor->energy_regen);
                    printf("Energy Storage: %d%%\n", Psensor->energy_storage);
                }
            }
        }

        scanf("%s", buffer);
    }
    // close file
    fclose(fptr);

    // free memory
    free_memory(Sensor_array, num_sensors);

    return 0;
}
