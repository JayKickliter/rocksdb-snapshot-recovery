#include <rocksdb/db.h>
#include <rocksdb/options.h>
#include <rocksdb/slice.h>
#include <rocksdb/snapshot.h>
#include <string>

const auto kDBPath{"/tmp/rocksdb_simple_example"};

int
main() {
    rocksdb::DB *    db;
    rocksdb::Options options;
    // create the DB if it's not already present
    options.create_if_missing = true;

    // open DB
    auto s = rocksdb::DB::Open(options, kDBPath, &db);
    assert(s.ok());

    // Put key-value
    s = db->Put(rocksdb::WriteOptions(), "key1", "value");
    assert(s.ok());
    std::string value;
    // get value
    s = db->Get(rocksdb::ReadOptions(), "key1", &value);
    assert(s.ok());
    assert(value == "value");

    {
        auto snapshot     = db->GetSnapshot();
        auto snapshot_seq = snapshot->GetSequenceNumber();
        printf("snapshot: %llu\n", snapshot_seq);
    }

    // atomically apply a set of updates
    {
        rocksdb::WriteBatch batch;
        batch.Delete("key1");
        batch.Put("key2", value);
        s = db->Write(rocksdb::WriteOptions(), &batch);
    }

    s = db->Get(rocksdb::ReadOptions(), "key1", &value);
    assert(s.IsNotFound());

    db->Get(rocksdb::ReadOptions(), "key2", &value);
    assert(value == "value");

    {
        rocksdb::PinnableSlice pinnable_val;
        db->Get(rocksdb::ReadOptions(),
                db->DefaultColumnFamily(),
                "key2",
                &pinnable_val);
        assert(pinnable_val == "value");
    }

    {
        std::string string_val;
        // If it cannot pin the value, it copies the value to its internal
        // buffer. The intenral buffer could be set during construction.
        rocksdb::PinnableSlice pinnable_val(&string_val);
        db->Get(rocksdb::ReadOptions(),
                db->DefaultColumnFamily(),
                "key2",
                &pinnable_val);
        assert(pinnable_val == "value");
        // If the value is not pinned, the internal buffer must have the value.
        assert(pinnable_val.IsPinned() || string_val == "value");
    }


    rocksdb::PinnableSlice pinnable_val;
    s = db->Get(rocksdb::ReadOptions(),
                db->DefaultColumnFamily(),
                "key1",
                &pinnable_val);
    assert(s.IsNotFound());
    // Reset PinnableSlice after each use and before each reuse
    pinnable_val.Reset();
    db->Get(rocksdb::ReadOptions(),
            db->DefaultColumnFamily(),
            "key2",
            &pinnable_val);
    assert(pinnable_val == "value");
    pinnable_val.Reset();
    // The Slice pointed by pinnable_val is not valid after this point

    db->Close();
    delete db;

    return 0;
}
