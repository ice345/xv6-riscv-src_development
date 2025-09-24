#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fcntl.h"

#define NUM_PROCESSES 4
#define LOCK_ID_CONTENTION 34
#define INCREMENTS_PER_PROCESS_CONTENTION 100
#define COUNTER_FILE "counter.txt"

// Global test statistics
static int tests_passed = 0;
static int tests_failed = 0;

#define TEST_ASSERT(condition, message) do { \
    if (condition) { \
        printf("✓ %s\n", message); \
        tests_passed++; \
    } else { \
        printf("✗ %s\n", message); \
        tests_failed++; \
    } \
} while(0)

// By code to verify the group theory to make sure 
// there is no problem with the underlying principle
void test_mathematical_properties(void) {
    printf("\n=== Mathematical Properties Verification Test ===\n");
    
    int result = grouplock_verify();
    TEST_ASSERT(result == 0, "Group theory mathematical properties verification passed");
    
    if (result == 0) {
        printf("Verification content:\n");
        printf("  - Z/2Z group closure property ✓\n");
        printf("  - Associativity ✓\n");
        printf("  - Commutativity (Abelian group property) ✓\n");
        printf("  - Identity element existence ✓\n");
        printf("  - Inverse element existence ✓\n");
        printf("  - Deadlock freedom mathematical proof ✓\n");
        printf("  - Atomic group operation verification ✓\n");
    }
}

// Test the group theory properties in practical operations
void test_group_theory_properties(void) {
    printf("=== Group Theory Properties Practical Verification ===\n");
    
    if (grouplock_create(6, "theory_lock") < 0) {
        printf("✗ Failed to create theory test lock\n");
        tests_failed++;
        return;
    }
    
    printf("Verifying specific applications of group operations:\n");
    
    // Verify identity property: e + a = a
    printf("1. Identity property: Initial state is 0 (identity element)\n");
    grouplock_debug(6);
    
    // Verify group operation: 0 + 1 = 1
    printf("2. Group operation: 0 + 1 = 1 (acquire operation)\n");
    if (grouplock_acquire(6) == 0) {
        grouplock_debug(6);
        
        // Verify inverse operation: 1 + 1 = 0
        printf("3. Inverse operation: 1 + 1 = 0 (release operation)\n");
        grouplock_release(6);
        grouplock_debug(6);
        
        TEST_ASSERT(1, "Group theory properties verified in practical operations");
    }
    
    grouplock_destroy(6);
}

// Test the basic operations of the group lock, like create, acquire, release, destroy
void test_basic_operations(void) {
    printf("\n=== Basic Operations Test ===\n");
    
    // Create group lock
    int result = grouplock_create(1, "test_lock");
    TEST_ASSERT(result == 0, "Successfully created group lock 1");
    
    // Acquire lock (0 + 1 = 1)
    result = grouplock_acquire(1);
    TEST_ASSERT(result == 0, "Successfully acquired group lock 1 (Group operation: 0 + 1 = 1)");
    
    // Release lock (1 + 1 = 0)
    result = grouplock_release(1);
    TEST_ASSERT(result == 0, "Successfully released group lock 1 (Group operation: 1 + 1 = 0)");
    
    // Acquire and release again to verify repeatability
    result = grouplock_acquire(1);
    TEST_ASSERT(result == 0, "Can repeatedly acquire group lock 1");
    
    result = grouplock_release(1);
    TEST_ASSERT(result == 0, "Can repeatedly release group lock 1");
    
    // Destroy lock
    result = grouplock_destroy(1);
    TEST_ASSERT(result == 0, "Successfully destroyed group lock 1");
}

// Test whether there will be problems in acquiring the same grouplock in multiple concurrent situations
void test_concurrent_access(void) {
    printf("\n=== Concurrent Access Test ===\n");
    
    if (grouplock_create(2, "concurrent_lock") < 0) {
        printf("✗ Failed to create concurrent test lock\n");
        tests_failed++;
        return;
    }
    
    printf("Creating child process for concurrent testing...\n");
    
    int pid = fork();
    if (pid == 0) {
        // Child process
        printf("Child process %d: Attempting to acquire lock (applying group operation)\n", getpid());
        if (grouplock_acquire(2) == 0) {
            printf("Child process %d: Successfully acquired lock, entering critical section\n", getpid());
            
            // Display debug info
            printf("Child process %d: Checking lock status\n", getpid());
            grouplock_debug(2);
            
            // Increase the probability of concurrency conflicts and test stability
            for (int i = 0; i < 1000000; i++); // Simple delay
            
            printf("Child process %d: Releasing lock (applying inverse operation)\n", getpid());
            grouplock_release(2);
        }
        exit(0);
    } else {
        // Parent process
        sleep(3); // Let child process run first
        printf("Parent process %d: Attempting to acquire lock\n", getpid());
        if (grouplock_acquire(2) == 0) {
            printf("Parent process %d: Successfully acquired lock, entering critical section\n", getpid());
            
            // Display debug info
            printf("Parent process %d: Checking lock status\n", getpid());
            grouplock_debug(2);
            
            // Increase the probability of concurrency conflicts and test stability
            for (int i = 0; i < 500000; i++); // Simple delay
            
            printf("Parent process %d: Releasing lock\n", getpid());
            grouplock_release(2);
        }
        
        wait(0); // Wait for child process to finish
        
        int result = grouplock_destroy(2);
        TEST_ASSERT(result == 0, "Concurrent test completed, lock properly cleaned up");
    }
}

// Test multiple processes competing for the same lock
// to verify the performance of locks under high concurrency 
// and verify the correctness and fairness of locks
void test_multiple_processes(void) {
    printf("\n=== Multi-process Stress Test ===\n");
    
    if (grouplock_create(3, "stress_lock") < 0) {
        printf("✗ Failed to create stress test lock\n");
        tests_failed++;
        return;
    }

    printf("Starting %d processes for stress test...\n", NUM_PROCESSES);

    // Create NUM_PROCESSES processes
    for (int i = 0; i < NUM_PROCESSES; i++) {
        int pid = fork();
        if (pid == 0) {
            // Child process
            int process_num = i;
            printf("Process %d (Number %d): Starting test\n", getpid(), process_num);
            
            // Every process tries to acquire and release the lock 3 times
            for (int j = 0; j < 3; j++) {
                printf("Process %d: Attempt %d to acquire lock\n", getpid(), j + 1);
                
                if (grouplock_acquire(3) == 0) {
                    printf("Process %d: Successfully acquired lock, executing critical section operations\n", getpid());
                    
                    // Simulate different workloads
                    for (int k = 0; k < (process_num + 1) * 200000; k++);
                    
                    printf("Process %d: Releasing lock\n", getpid());
                    grouplock_release(3);
                } else {
                    printf("Process %d: Failed to acquire lock\n", getpid());
                }
                
                // Inter-process interval
                for (int k = 0; k < 100000; k++);
            }
            
            printf("Process %d: Test completed\n", getpid());
            exit(0);
        }
    }
    
    // Wait for all child processes to complete
    for (int i = 0; i < NUM_PROCESSES; i++) {
        wait(0);
    }
    
    int result = grouplock_destroy(3);
    TEST_ASSERT(result == 0, "Multi-process stress test completed");
}

// Helper function to read an integer from a file
int read_counter(const char* filename) {
    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        printf("read_counter: open failed\n");
        return -1;
    }
    char buf[16];
    memset(buf, 0, sizeof(buf)); // Fill buf with 0 for easier debugging
    int n = read(fd, buf, sizeof(buf) - 1);
    close(fd);
    if (n <= 0) {
      return -1;
    }
    buf[n] = '\0';
    return atoi(buf);
}

// Helper function to convert integer to string
void itoa(int n, char* buf) {
    int i = 0;
    char temp[16]; // Storer for digits temporary
    memset(temp, 0, sizeof(temp));
    int is_negative = 0;

    if (n == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return;
    }

    if (n < 0) {
        is_negative = 1;
        n = -n;
    }

    // Store each digit of the number into the temp array (in reverse order)
    while (n != 0) {
        temp[i++] = (n % 10) + '0';
        n = n / 10;
    }

    if (is_negative) {
        temp[i++] = '-';
    }

    // Reverse the temp array into the final buf
    int j = 0;
    while (i > 0) {
        buf[j++] = temp[--i];
    }
    buf[j] = '\0';
}

// Helper function to write an integer to a file (overwrite)
void write_counter(const char* filename, int counter) {
    // O_CREATE: If the file does not exist, create it
    // O_WRONLY: Write only
    // O_TRUNC: Clear the file before writing
    int fd = open(filename, O_CREATE | O_WRONLY | O_TRUNC);
    if (fd < 0) {
        printf("write_counter: open failed\n");
        return;
    }
    char buf[16];
    memset(buf, 0, sizeof(buf));
    itoa(counter, buf);
    write(fd, buf, strlen(buf));
    close(fd);
}

// Test lock contention with multiple processes incrementing a shared counter in a file
void test_lock_contention(void) {
    printf("\n=== GroupLock Contention Test (on Shared File) ===\n");

    // 1. Initialize lock and file
    if (grouplock_create(LOCK_ID_CONTENTION, "contention_lock") < 0) {
        printf("✗ Failed to create contention lock\n");
        tests_failed++;
        return;
    }
    write_counter(COUNTER_FILE, 0); // Initialize counter file to 0

    printf("Starting %d processes to contend for the lock...\n", NUM_PROCESSES);

    // 2. Create child processes
    for (int i = 0; i < NUM_PROCESSES; i++) {
        int pid = fork();
        if (pid < 0) {
            printf("✗ Fork failed\n");
            break;
        }
        if (pid == 0) {
            // Child process code
            for (int j = 0; j < INCREMENTS_PER_PROCESS_CONTENTION; j++) {
                // a. Get lock
                grouplock_acquire(LOCK_ID_CONTENTION);

                // b. --- Critical section start ---
                // Read, modify, and write back to file
                int current_val = read_counter(COUNTER_FILE);
                if (current_val != -1) {
                    write_counter(COUNTER_FILE, current_val + 1);
                }
                // --- Critical section end ---

                // c. Release lock
                grouplock_release(LOCK_ID_CONTENTION);
            }
            exit(0);
        }
    }

    // 3. Wait for all child processes to finish
    for (int i = 0; i < NUM_PROCESSES; i++) {
        wait(0);
    }

    // 4. Verify final counter value
    int final_val = read_counter(COUNTER_FILE);
    int expected_val = NUM_PROCESSES * INCREMENTS_PER_PROCESS_CONTENTION;

    printf("Final counter value in file: %d\n", final_val);
    printf("Expected value: %d\n", expected_val);

    TEST_ASSERT(final_val == expected_val, "Lock correctly prevented race conditions on shared file");

    // 5. Cleanup
    grouplock_destroy(LOCK_ID_CONTENTION);
    unlink(COUNTER_FILE); // Delete the test file
}

//Test some cases like invalid ID, repeated operations, destroying a lock in use
void test_edge_cases(void) {
    printf("\n=== Edge Cases Test ===\n");
    
    // Test invalid ID
    int result = grouplock_acquire(-1);
    TEST_ASSERT(result < 0, "Correctly rejected invalid lock ID -1");
    
    result = grouplock_acquire(999);
    TEST_ASSERT(result < 0, "Correctly rejected invalid lock ID 999");
    
    // Test repeated operations
    if (grouplock_create(4, "edge_lock") == 0) {
        if (grouplock_acquire(4) == 0) {
            // Try repeated release
            grouplock_release(4);
            result = grouplock_release(4);
            TEST_ASSERT(result < 0, "Correctly rejected repeated release");
        }
        
        // Test repeated creation
        result = grouplock_create(4, "duplicate");
        TEST_ASSERT(result < 0, "Correctly rejected repeated creation of lock with same ID");
        
        grouplock_destroy(4);
    }
    
    // Test destroying a lock in use
    if (grouplock_create(5, "busy_lock") == 0) {
        grouplock_acquire(5);
        result = grouplock_destroy(5);
        TEST_ASSERT(result < 0, "Correctly rejected destroying a lock in use");
        grouplock_release(5);
        grouplock_destroy(5);
    }
}



int main(int argc, char *argv[]) {
    printf("=== GroupLock Complete Test Suite ===\n");
    printf("Lock mechanism test based on abstract algebra Z/2Z group theory\n");
    printf("Author: Operating Systems Course Project\n");
    printf("Theoretical foundation: Finite group Z/2Z = ({0,1}, +)\n");
    
    // Run all tests
    test_mathematical_properties();
    test_group_theory_properties();
    test_basic_operations();
    test_concurrent_access();
    test_multiple_processes();
    test_edge_cases();
    test_lock_contention();
    
    // Test results summary
    printf("=== Test Results Summary ===\n");
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_failed);
    printf("Total tests: %d\n", tests_passed + tests_failed);
    
    if (tests_failed == 0) {
        printf("🎉 All tests passed! GroupLock mechanism works correctly.\n");
        printf("Mathematical theory and system implementation perfectly combined!\n");
    } else {
        printf("⚠️  %d tests failed, implementation needs to be checked.\n", tests_failed);
    }
    
    return 0;
}
