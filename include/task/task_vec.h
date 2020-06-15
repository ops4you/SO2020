#ifndef TASK_TASK_VEC_H
#define TASK_TASK_VEC_H

#include <stdbool.h>
#include <stddef.h>

/**
 * If set to @p 1, perform runtime assertions on possibly invalid function
 * arguments.
 */
#define TASK_VEC_RUNTIME_ASSERTS 0

struct Task;

/**
 * A dynamic array of Tasks capable of shrinking and expanding.
 * Provides random access, as well as contiguous iterators to the beginning and
 * the end.
 */
typedef struct TaskVec {
    struct Task* buf;  //!< A continuous, dynamically allocated buffer of Tasks.
    size_t len; //!< The length of the buffer.
    size_t cap; //!< The max capacity of Tasks it can store at a given moment.
} TaskVec;

/**
 * Creates an empty TaskVec.
 * The TaskVec must later be passed to <tt>tvec_drop()</tt>.
 * <tt>O(1)</tt> complexity.
 * @param init (output parameter) the address of the TaskVec to initialize.
 * <b>Must not be @p NULL.</b>
 * @return a pointer to the initialized TaskVec with address @p init.
 */
TaskVec* tvec_new(TaskVec* init);

/**
 * Creates a TaskVec with the provided capacity.
 * If <tt>capacity * sizeof(Task)</tt> overflows, or if memory allocation fails,
 * @p NULL is returned.
 * The TaskVec must later be passed to <tt>tvec_drop()</tt>.
 * <tt>O(malloc(capacity * sizeof(Task)))</tt> complexity.
 * @param init (output parameter) the address of the TaskVec to initialize.
 * <b>Must not be @p NULL.</b>
 * @param capacity the desired initial capacity of the TaskVec.
 * @return a pointer to the initialized TaskVec with address @p init and
 * capacity @p capacity.
 */
TaskVec* tvec_with_cap(TaskVec* init, size_t capacity);

/**
 * Deallocates the storage associated with a TaskVec.
 * Reading from pointers to Tasks previously owned by the TaskVec is undefined.
 * <tt>O(free(self->buf))</tt> complexity.
 * @param self the address of the TaskVec whose store shall be deallocated.
 * <b>Must not be @p NULL.</b>
 */
void tvec_drop(TaskVec* self);

/**
 * Returns the TaskVec's length.
 * <tt>O(1)</tt> complexity.
 * @param self the address of the TaskVec whose length shall be returned.
 * <b>Must not be @p NULL.</b>
 * @return the TaskVec's length.
 */
size_t tvec_len(TaskVec const* self);

/**
 * Returns the TaskVec's capacity.
 * <tt>O(1)</tt> complexity.
 * @param self the address of the TaskVec whose capacity shall be returned.
 * <b>Must not be @p NULL.</b>
 * @return the TaskVec's capacity.
 */
size_t tvec_cap(TaskVec const* self);

/**
 * Checks if the TaskVec is empty, i.e. it contains no Tasks.
 * <tt>O(1)</tt> complexity.
 * @param self the address of the TaskVec to check whether it's empty.
 * <b>Must not be @p NULL.</b>
 * @return @p true if it's empty, @p false otherwise.
 */
bool tvec_is_empty(TaskVec const* self);

/**
 * Checks if the TaskVec is at maximum capacity, i.e. if its length is equal to
 * its capacity.
 * <tt>O(1)</tt> complexity.
 * @param self the address of the TaskVec to check whether it's at maximum
 * capacity. <b>Must not be @p NULL.</b>
 * @return @p true if it's at maximum capacity, @p false otherwise.
 */
bool tvec_is_at_max_cap(TaskVec const* self);

/**
 * Shrinks the TaskVec as much as possible, such that after this call,
 * <tt>tvec_len(self) == tvec_cap(self)</tt>.
 * If the TaskVec is already at maximum capacity, does nothing. If it isn't, a
 * reallocation of <tt>tvec_len(self) * sizeof(Task)</tt> is attempted.
 * If this reallocation succeeds, @p true is returned, otherwise @p false is
 * returned and the TaskVec is left unchanged.
 * <tt>O(realloc(tvec_len(self) * sizeof(Task)))</tt> complexity.
 * @param self the address of the TaskVec to shrink. <b>Must not be @p NULL.</b>
 * @return @p true if the TaskVec was shrunk successfully, @p false otherwise.
 */
bool tvec_shrink_to_fit(TaskVec* self);

/**
 * Shrinks the TaskVec to <tt>min(tvec_len(self), min_capacity)</tt>. If this
 * size is equal to the TaskVec's capacity, does nothing. If it isn't, a
 * reallocation of this size is attempted.
 * If this reallocation succeeds, @p true is returned, otherwise @p false is
 * returned and the TaskVec is left unchanged.
 * <tt>O(realloc(min(tvec_len(self), min_capacity) * sizeof(Task)))</tt>
 * complexity.
 * @param self the address of the TaskVec to shrink. <b>Must not be @p NULL.</b>
 * @return @p true if the TaskVec was shrunk successfully, @p false otherwise.
 */
bool tvec_shrink_to(TaskVec* self, size_t min_capacity);

/**
 * Returns a readonly iterator to the first Task. If the TaskVec is empty, this
 * iterator is equal to tvec_end(self).
 * <tt>O(1)</tt> complexity.
 * @param self the address of the TaskVec whose beginning iterator shall be
 * returned. <b>Must not be @p NULL.</b>
 * @return a readonly iterator to the first Task.
 */
struct Task const* tvec_begin(TaskVec const* self);

/**
 * Returns a mutable iterator to the first Task. If the TaskVec is empty, this
 * iterator is equal to tvec_end(self).
 * <tt>O(1)</tt> complexity.
 * @param self the address of the TaskVec whose beginning iterator shall be
 * returned. <b>Must not be @p NULL.</b>
 * @return a mutable iterator to the first Task.
 */
struct Task* tvec_begin_mut(TaskVec* self);

/**
 * Returns an iterator to the end of the TaskVec, i.e. a pointer to the
 * past-the-end Task.
 * <tt>O(1)</tt> complexity.
 * @param self the address of the TaskVec whose end iterator shall be returned.
 * <b>Must not be @p NULL.</b>
 * @return an iterator to the end of the TaskVec.
 */
struct Task const* tvec_end(TaskVec const* self);

/**
 * Returns a readonly iterator to the Task at index @p idx in the TaskVec.
 * If <tt>idx >= tvec_len(self)</tt>, @p NULL is returned.
 * <tt>O(1)</tt> complexity.
 * @param self the address of the TaskVec from which to return the iterator.
 * <b>Must not be @p NULL.</b>
 * @param idx the index of the Task whose iterator is returned.
 * @return a readonly iterator to the Task at index @p idx, or @p NULL if
 * <tt>idx >= tvec_len(self)</tt>.
 */
struct Task const* tvec_at(TaskVec const* self, size_t idx);

/**
 * Returns a mutable iterator to the Task at index @p idx in the TaskVec.
 * If <tt>idx >= tvec_len(self)</tt>, @p NULL is returned.
 * <tt>O(1)</tt> complexity.
 * @param self the address of the TaskVec from which to return the iterator.
 * <b>Must not be @p NULL.</b>
 * @param idx the index of the Task whose iterator is returned.
 * @return a mutable iterator to the Task at index @p idx, or @p NULL if
 * <tt>idx >= tvec_len(self)</tt>.
 */
struct Task* tvec_at_mut(TaskVec* self, size_t idx);

/**
 * Returns a readonly iterator to the Task with id @p tid in the TaskVec.
 * If no Task with such id exists, @p NULL is returned.
 * If @p opt_idx isn't @p NULL, the index of the Task in the TaskVec is stored
 * to it, if a Task was found.
 * <tt>O(tvec_len(self))</tt> complexity.
 * @param self the address of the TaskVec from which to return the iterator.
 * <b>Must not be @p NULL.</b>
 * @param tid the Task id of the Task whose iterator is returned.
 * @param opt_idx (output parameter) optional address of a size type to which is
 * stored the Task index in the TaskVec, if a Task was found. If @p NULL, it is
 * unused.
 * @return a readonly iterator to the Task with id @p tid, or @p NULL if
 * such Task doesn't exist.
 */
struct Task const* tvec_search_by_tid(
    TaskVec const* restrict self,
    size_t tid,
    size_t* restrict opt_idx
);

/**
 * Returns a mutable iterator to the Task with id @p tid in the TaskVec.
 * If no Task with such id exists, @p NULL is returned.
 * If @p opt_idx isn't @p NULL, the index of the Task in the TaskVec is stored
 * to it, if a Task was found.
 * <tt>O(tvec_len(self))</tt> complexity.
 * @param self the address of the TaskVec from which to return the iterator.
 * <b>Must not be @p NULL.</b>
 * @param tid the Task id of the Task whose iterator is returned.
 * @param opt_idx (output parameter) optional address of a size type to which is
 * stored the Task index in the TaskVec, if a Task was found. If @p NULL, it is
 * unused.
 * @return a mutable iterator to the Task with id @p tid, or @p NULL if
 * such Task doesn't exist.
 */
struct Task* tvec_search_by_tid_mut(
    TaskVec* restrict self,
    size_t tid,
    size_t* restrict opt_idx
);

/**
 * Pushes a Task to the end of the TaskVec. The Task is shallow copied to the
 * end of the TaskVec by means of the assignment operator @p =.
 * If a reallocation occurs, reading from pointers to Tasks previously owned by
 * the TaskVec is undefined.
 * If a reallocation occurs and fails, reallocations of decreasingly smaller
 * sizes are attempted. If even then reallocation fails, @p false is returned.
 * Amortized <tt>O(1)</tt> complexity.
 * @param self the address of the TaskVec to which the Task is pushed.
 * <b>Must not be @p NULL.</b>
 * @param task the address of the Task to push to the TaskVec.
 * <b>Must not be @p NULL.</b>
 * @return @p true if the Task was successfully pushed, @p false otherwise.
 */
bool tvec_push(TaskVec* restrict self, struct Task* restrict task);

/**
 * Inserts a Task at index @p idx in the TaskVec. The Task is shallow copied to
 * the TaskVec by means of the assignment operator @p =.
 * If <tt>idx > tvec_len(self)</tt>, no Task is inserted, and @p false is
 * returned.
 * If a reallocation occurs, reading from pointers to Tasks previously owned by
 * the TaskVec is undefined.
 * If a reallocation occurs and fails, reallocations of decreasingly smaller
 * sizes are attempted. If even then reallocation fails, @p false is returned.
 * Amortized <tt>O(tvec_len(self) - idx)</tt> complexity.
 * @param self the address of the TaskVec to which the Task is inserted.
 * <b>Must not be @p NULL.</b>
 * @param task the address of the Task to insert in the TaskVec.
 * <b>Must not be @p NULL.</b>
 * @param idx the index in the TaskVec at which to insert the Task.
 * <b>Must be <= <tt>tvec_len(self)</tt>.</b>
 * @return @p true if the Task was successfully inserted, @p false otherwise.
 */
bool tvec_insert_at(
    TaskVec* restrict self,
    struct Task* restrict task,
    size_t idx
);

/**
 * Pops a Task from the end of the TaskVec.
 * If @p opt_task isn't @p NULL, the popped Task is shallow copied to it by
 * means of the assignment operator @p =.
 * <tt>O(1)</tt> complexity.
 * @param self the address of the TaskVec from which to pop a Task.
 * <b>Must not be @p NULL.</b>
 * @param opt_task (output parameter) optional address of a Task to which the
 * popped Task may be shallow copied to. If @p NULL, it is unused.
 * @return @p NULL if the TaskVec is empty. If it isn't, and @p opt_task is not
 * @p NULL, it's initalized with the popped Task and returned, otherwise,
 * @p NULL is returned.
 */
struct Task* tvec_pop(TaskVec* restrict self, struct Task* restrict opt_task);

/**
 * Removes a Task at index @p idx in the TaskVec.
 * The Task is actually overridden with the last Task, which causes any previous
 * order in the TaskVec to be discarded.
 * If <tt>idx >= tvec_len(self)</tt>, no Task is removed, and @p false is
 * returned.
 * <tt>O(1)</tt> complexity.
 * @param self the address of the TaskVec from which the Task is removed.
 * <b>Must not be @p NULL.</b>
 * @param idx the index in the TaskVec from which to remove the Task.
 * <b>Must be < <tt>tvec_len(self)</tt>.</b>
 * @return @p true if the Task was successfully removed, @p false otherwise.
 */
bool tvec_rm_at(TaskVec* self, size_t idx);

/**
 * Removes a Task at index @p idx in the TaskVec.
 * If <tt>idx >= tvec_len(self)</tt>, no Task is removed, and @p false is
 * returned.
 * <tt>O(tvec_len(self) - idx)</tt> complexity.
 * @param self the address of the TaskVec from which the Task is removed.
 * <b>Must not be @p NULL.</b>
 * @param idx the index in the TaskVec from which to remove the Task.
 * @return @p true if the Task was successfully removed, @p false otherwise.
 */
bool tvec_rm_ord_at(TaskVec* self, size_t idx);

/**
 * Removes a Task from the TaskVec by Task id.
 * If no Task with such id exists, no Task is removed, and @p false is
 * returned.
 * <tt>O(tvec_len(self))</tt> complexity.
 * @param self the address of the TaskVec from which the Task is removed.
 * <b>Must not be @p NULL.</b>
 * @param tid the task id of the Task to remove from the TaskVec.
 * @return @p true if the Task was successfully removed, @p false otherwise.
 */
bool tvec_rm_by_tid(TaskVec* self, size_t tid);

#endif  // TASK_TASK_VEC_H
