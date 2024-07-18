#include <assert.h>
#include "ring_buffer.h" // replace with your actual header file
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
void put(key_type k, value_type v) ;
value_type get(key_type k) ;
void init_kvstore(int size);

void test_put_get() {
    key_type key = 1;
    value_type value = 100;

    // Test put function
    put(key, value);

    // Test get function
    value_type retrieved_value = get(key);
    assert(retrieved_value == value);
}
void test_collision() {
    key_type key1 = 1;
    key_type key2 = 11; // Assuming your hash function is key mod size, these two keys will collide in a hashtable of size 10
    value_type value1 = 100;
    value_type value2 = 200;

    // Put two values that will collide
    put(key1, value1);
    put(key2, value2);

    // Get the values and check they're correct
    value_type retrieved_value1 = get(key1);
    value_type retrieved_value2 = get(key2);
    assert(retrieved_value1 == value1);
    assert(retrieved_value2 == value2);
}

int main() {
    init_kvstore(10); // Initialize the key-value store with size 10
    test_put_get();
    test_collision();
    printf("All tests passed!\n");
    return 0;
}
