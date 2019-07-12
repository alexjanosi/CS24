#ifndef RECORD_H
#define RECORD_H


/*! Total number of records to sort. */
#define NUM_RECORDS 25000000

/*! Size of the entire record, in bytes. */
#define RECORD_SIZE 128

/*! Size of the key for each record, in bytes. */
#define KEY_SIZE 32

/*! The remainder of the record is "value" data. */
#define VALUE_SIZE (RECORD_SIZE - KEY_SIZE)

/*!
 * Number of characters from which keys are generated.  For example,
 * if this is 3 then keys will be comprised of the characters "ABC".
 */
#define KEY_VALUES 10


/*! Represents a single record in the program. */
typedef struct record_t {
    char key[KEY_SIZE];       //!< The key for the record
    char value[VALUE_SIZE];   //!< The value for the record
} record_t;


record_t * generate_records(uint32_t num_records);


#endif /* RECORD_H */

