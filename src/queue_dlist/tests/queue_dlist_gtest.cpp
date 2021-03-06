#include <gtest/gtest.h>
#include <dl_queue.h>

/*
 * Helper Functions for testing
 */
// Creates a simple payload for the linked list
int * get_payload(int num)
{
    int * ptr = (int * )malloc(sizeof(int));
    *ptr = num;
    return ptr;
}

// frees the payload inserted into the linked list
void free_payload(void * data)
{
    free(data);
}

// function for comparing nodes
queue_status_t compare_payloads(void * data1, void * data2)
{
    if (*(int*)data1 == *(int*)data2)
    {
        return Q_MATCH;
    }
    return Q_NO_MATCH;
}
/*
 * //end of Helper Functions for testing
 */

// Simple test to get up and running
TEST(QueueTest, TestAllocation)
{
    queue_t * queue = queue_init(10, compare_payloads);
    ASSERT_NE(queue, nullptr);
    queue_destroy(queue);
}

/*
 * Test fixture to do more complicated testing
 *
 * This fixture creates a dlist of 10 items with 10 strings
 */
class DQueueTestFixture : public ::testing::Test
{
 public:
    queue_t * queue;
    void * payload_first;
    void * payload_last;
    int length = 10;

 protected:
    void SetUp() override
    {
        queue = queue_init(length, compare_payloads);
        for (int i = 0; i < length; i++)
        {
            void * payload = get_payload(i);
            if (0 == i)
            {
                payload_first = payload;
            }
            else if (length - 1 == i)
            {
                payload_last = payload;
            }
            queue_enqueue(queue, payload);
        }
    }
    void TearDown() override
    {
        queue_destroy_free(queue, free_payload);
    }
};


// Test ability to pop from the queue
TEST_F(DQueueTestFixture, TestPopQueue)
{
    EXPECT_EQ(this->length, queue_length(this->queue));
    int * value = (int * )queue_dequeue(this->queue);
    EXPECT_EQ(*value, *(int * )this->payload_first);

    EXPECT_EQ(this->length - 1, queue_length(this->queue));
    free(value);
}

// Test if the queue successfully stops dequeue when it is empty
TEST_F(DQueueTestFixture, TestPopOverRun)
{
    EXPECT_EQ(this->length, queue_length(this->queue));

    for (int i = 0; i < this->length; i++)
    {
        free(queue_dequeue(this->queue));
    }

    EXPECT_EQ(0, queue_length(this->queue));
    ASSERT_EQ(queue_dequeue(this->queue), nullptr);
}

// Test the inability to enqueue when the queue is full
TEST_F(DQueueTestFixture, TestEnqueueLimit)
{
    EXPECT_EQ(this->length, queue_length(this->queue));

    void * payload = get_payload(20);
    queue_status_t status = queue_enqueue(queue, payload);

    EXPECT_EQ(status, Q_FAILURE);
    EXPECT_EQ(this->length, queue_length(this->queue));
    free(payload);
}

// Test the ability to find a node by its value
TEST_F(DQueueTestFixture, TestFindMethod)
{

    void * payload = get_payload((this->length / 2));
    void * queue_node = queue_get_by_value(this->queue, payload);
    ASSERT_NE(queue_node, nullptr);

    queue_status_t status = compare_payloads(payload, queue_node);
    EXPECT_EQ(status, Q_MATCH);

    free(payload);
}

// Test ability to find a node by its index
TEST_F(DQueueTestFixture, TestFindIndexMethod)
{
    int index = this->length / 2;
    void * queue_node = queue_get_by_index(this->queue, index);
    ASSERT_NE(queue_node, nullptr);

    // Payloads are the same as their index so index should equal the queue_node
    EXPECT_EQ(*(int*)queue_node, index);
}

// Test ability to delete an item from the queue without dequeue
TEST_F(DQueueTestFixture, TestDeleteItem)
{
    void * payload = get_payload((this->length / 2));
    void * queue_node = queue_remove(this->queue, payload);
    ASSERT_NE(queue_node, nullptr);

    queue_status_t status = compare_payloads(payload, queue_node);
    EXPECT_EQ(status, Q_MATCH);

    free(payload);
    free(queue_node);
}
