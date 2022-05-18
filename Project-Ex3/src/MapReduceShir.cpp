//#include "MapReduceFramework.h"
//#include "Barrier.h"
//
//#import <pthread.h>
//#include <iostream>
//#include <atomic>
//#include <algorithm>
//#include <unistd.h>
//
////-------------------------------------------ERROR MSG-----------------------------------------//
//#define PTHREAD_CREATE_FAILED "system error: unable to create thread\n"
//#define PTHREAD_LOCK_MUTEX "system error: unable to lock mutex\n"
//#define PTHREAD_UNLOCK_MUTEX "system error: unable to unlock mutex\n"
//#define ERROR_INITIALIZED_JOB_CONTEXT "error: init job context failed\n"
//#define PTHREAD_JOIN_FAIL "system error: pthread_join failed\n"
//#define ERR_DESTROY_THREAD "system error: destroy thread context's mutex\n"
//#define ERR_DESTORY_CONTEXT "system error: destroy JobContext's mutex\n"
//
//
//typedef struct JobContext;
//
///**
// *A struct of thread job that allows us to split the input values between the threads
// */
//typedef struct threadJobContext
//{
//    std::vector<IntermediatePair> threadVecIntermediate;
//    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
//    JobContext *jobContext;
//} threadJobContext;
//
///***
// * A struct of job that contains all the parameters which are relevant to the job.
// */
//typedef struct JobContext
//{
//    JobState jobState;                 /** state of job **/
//    const MapReduceClient &client;
//    const InputVec &inputVec;         /** input vector **/
//    OutputVec &outputVec;             /** output vector **/
//    IntermediateMap intermediateMap;
//    pthread_t *threadsPool;             /** all the threads in the program **/
//    std::vector<threadJobContext> threadsContext;
//    std::vector<K2 *> keys;             /** key from stage2 **/
//    int numThread = 0;
//    int numTaskDone = 0;
//    int numReduceDone = 0;
//    std::atomic<int> numsShuffleDone = {0};
//    std::atomic<int> mapCounter{0};
//    std::atomic<int> curThread{0};
//    std::atomic<int> mapStageDone{0};
//    std::atomic<int> reduceCounter = {0};
//    pthread_mutex_t pMutex = PTHREAD_MUTEX_INITIALIZER;
//    Barrier *barrierShuffle;
//
//    JobContext(const MapReduceClient &client, const InputVec &inputVec,
//               OutputVec &outputVec)
//            : inputVec(inputVec), client(client), outputVec(outputVec)
//    {}
//
//    /** * destructor */
//    ~JobContext()
//    {
//        if (pthread_mutex_destroy(&pMutex) != 0)
//        {
//            std::cerr << ERR_DESTORY_CONTEXT;
//            exit(EXIT_FAILURE);
//        }
//        for (int i = 0; i < numThread; i++)
//        {
//
//            if (pthread_mutex_destroy(&threadsContext[i].mutex) != 0)
//            {
//                std::cerr <<  ERR_DESTROY_THREAD;
//                exit(EXIT_FAILURE);
//            }
//        }
//        delete barrierShuffle;
//        delete[] threadsPool;
//
//    }
//} JobContext;
//
//
///**
// * In this phase each thread reads a key k2 and calls the reduce function using the key and it’s
// * value from the IntermediateMap. The reduce function in turn will call the emit3 function to
// * output
//( k3 , v3 ) pairs which can be inserted directly to the output vector.
// * @param context - JobConext
// */
//void *reducePhase(void *context)
//{
//    threadJobContext *reduceContext = (threadJobContext *) context;
//    int old_value = reduceContext->jobContext->reduceCounter++;
//    while (old_value < reduceContext->jobContext->keys.size())
//    {
//        reduceContext->jobContext->client.reduce(
//                reduceContext->jobContext->keys[old_value],
//                reduceContext->jobContext->intermediateMap[reduceContext->jobContext->keys[old_value]],
//                reduceContext);
//        int res = pthread_mutex_lock(&reduceContext->jobContext->pMutex);
//        if (res)
//        {
//            std::cerr << PTHREAD_LOCK_MUTEX;
//            exit(EXIT_FAILURE);
//        }
//        reduceContext->jobContext->numReduceDone++; // thread took a task
//        reduceContext->jobContext->jobState.percentage =
//                (float) reduceContext->jobContext->numReduceDone /
//                reduceContext->jobContext->intermediateMap.size() * 100;
//        res = pthread_mutex_unlock(&reduceContext->jobContext->pMutex);
//        if (res)
//        {
//            std::cerr << PTHREAD_UNLOCK_MUTEX;
//            exit(EXIT_FAILURE);
//        }
//        old_value = reduceContext->jobContext->reduceCounter++;
//    }
//    return reduceContext;
//}
//
///**
// * This phase reads the Map phase output and combines them into a single IntermediateMap
// * @param context - JobHandle
// */
//void *shufflePhase(void *context)
//{
//    threadJobContext *shuffleContext = (threadJobContext *) context;
//    int iThread = 0;
//    size_t totalTaskToShuffle = 0;
//    while (!((shuffleContext->jobContext->jobState.stage == SHUFFLE_STAGE) &&
//             (shuffleContext->jobContext->jobState.percentage >= 100.0)))
//        //as long as the shuffle is running
//    {
//        if ((shuffleContext->jobContext->mapStageDone >=
//             (shuffleContext->jobContext->numThread - 1)) &&
//            (shuffleContext->jobContext->jobState.stage != SHUFFLE_STAGE
//            )) //change statue to shuffle
//        {
//            for (int i = 0; i < shuffleContext->jobContext->numThread - 1; i++)
//                //happens once, when map stage done, calc the rest of the vector that were not shuffled
//            {
//                totalTaskToShuffle +=
//                        shuffleContext->jobContext->threadsContext[i].threadVecIntermediate.size();
//            }
//            totalTaskToShuffle += shuffleContext->jobContext->numsShuffleDone;
//            //add to the totalToShuffle the ones that already shuffled
//            int res1 = pthread_mutex_lock(
//                    &shuffleContext->jobContext->pMutex);
//            if (res1)
//            {
//                std::cerr << PTHREAD_LOCK_MUTEX;
//                exit(EXIT_FAILURE);
//            }
//            if (totalTaskToShuffle != 0)
//            {
//                float perc = 100 *
//                             (float) shuffleContext->jobContext->numsShuffleDone /
//                             totalTaskToShuffle;
//                shuffleContext->jobContext->jobState = {SHUFFLE_STAGE, perc};
//
//            }
//            else
//            {
//                shuffleContext->jobContext->jobState = {SHUFFLE_STAGE,
//                                                        (float) 0};
//
//            }
//            res1 = pthread_mutex_unlock(&shuffleContext->jobContext->pMutex);
//            if (res1)
//            {
//                std::cerr << PTHREAD_UNLOCK_MUTEX;
//                exit(EXIT_FAILURE);
//            }
//
//        }
//        int res1 = pthread_mutex_lock(
//                &shuffleContext->jobContext->threadsContext[iThread].mutex);
//        if (res1)
//        {
//            std::cerr << PTHREAD_LOCK_MUTEX;
//            exit(EXIT_FAILURE);
//        }
//        if (!shuffleContext->jobContext->threadsContext[iThread].threadVecIntermediate.empty())
//        {
//            for(auto pair:shuffleContext->jobContext->threadsContext[iThread].threadVecIntermediate)
//            {
//                shuffleContext->jobContext->intermediateMap[pair.first].push_back(pair.second);
//
//            }
//            shuffleContext->jobContext->numsShuffleDone +=
//                    shuffleContext->jobContext->threadsContext[iThread].threadVecIntermediate.size();
//            if (shuffleContext->jobContext->jobState.stage == SHUFFLE_STAGE)
//            {
//                shuffleContext->jobContext->jobState.percentage =
//                        (float) shuffleContext->jobContext->numsShuffleDone /
//                        totalTaskToShuffle *
//                        100;
//            }
//            shuffleContext->jobContext->threadsContext[iThread].threadVecIntermediate.clear();
//        }
//        res1 = pthread_mutex_unlock(
//                &shuffleContext->jobContext->threadsContext[iThread].mutex);
//        if (res1)
//        {
//            std::cerr << PTHREAD_UNLOCK_MUTEX;
//            exit(EXIT_FAILURE);
//        }
//        iThread = (iThread + 1) % (shuffleContext->jobContext->numThread -
//                                   1); //in case there are more tasks than threads
//    }
//    for (auto const &v : shuffleContext->jobContext->intermediateMap)
//    {
//        shuffleContext->jobContext->keys.push_back(v.first);
//    }
//    int res1 = pthread_mutex_lock
//            (&shuffleContext->jobContext->pMutex);
//    if (res1)
//    {
//        std::cerr << PTHREAD_LOCK_MUTEX;
//        exit(EXIT_FAILURE);
//    }
//    shuffleContext->jobContext->jobState = {REDUCE_STAGE, 0};
//
//    res1 = pthread_mutex_unlock(&shuffleContext->jobContext->pMutex);
//    if (res1)
//    {
//        std::cerr << PTHREAD_UNLOCK_MUTEX;
//        exit(EXIT_FAILURE);
//    }
//    shuffleContext->jobContext->barrierShuffle->barrier(); //make sure that shuffle&map  have done.
//    reducePhase(shuffleContext);
//    return shuffleContext;
//}
//
//
///**
// * In this phase each thread reads pairs of ( k1 , v1 ) from the input vector and calls the map
// * function on each of them. The map function in turn will call the emit2 function
// * to output ( k2, v2 ) pairs.
// * @param context - JobContext
// */
//void *mapPhase(void *context)
//{
//    threadJobContext *threadContext = (threadJobContext *) context;
//
//    int index = threadContext->jobContext->curThread++;
//    int old_value = threadContext->jobContext->mapCounter++;
//
//    while (old_value < threadContext->jobContext->inputVec.size())
//        //splitting the input values between the threads
//    {
//        threadContext->jobContext->client.map(
//                (threadContext->jobContext->inputVec[old_value].first),
//                (threadContext->jobContext->inputVec)[old_value].second,
//                threadContext); // Actual mapping
//
//        int res = pthread_mutex_lock(&threadContext->jobContext->pMutex);
//        if (res)
//        {
//            std::cerr << PTHREAD_LOCK_MUTEX;
//            exit(EXIT_FAILURE);
//        }
//
//        // Critical section starts
//        threadContext->jobContext->numTaskDone++; // thread took a task
//        threadContext->jobContext->jobState.percentage =
//                (float) threadContext->jobContext->numTaskDone /
//                threadContext->jobContext->inputVec.size() * 100;
//        // Critical section ends
//
//        res = pthread_mutex_unlock(&threadContext->jobContext->pMutex);
//        if (res)
//        {
//            std::cerr << PTHREAD_UNLOCK_MUTEX;
//            exit(EXIT_FAILURE);
//        }
//        old_value = threadContext->jobContext->mapCounter++;
//
//    } //All tasks were distributed by the threads
//
//    ++threadContext->jobContext->mapStageDone;
//
//    threadContext->jobContext->barrierShuffle->barrier(); //make sure that all tasks were done
//
//    reducePhase(threadContext);
//
//    return threadContext;
//}
//
///**
// * This function starts running the MapReduce algorithm
// * @param context - JobHandle
// */
//JobHandle
//startMapReduceJob(const MapReduceClient &client, const InputVec &inputVec,
//                  OutputVec &outputVec,
//                  int multiThreadLevel)
//{
//    int pt_res = 0;
//    JobContext *job;
//    try
//    {
//        job = new JobContext(client, inputVec, outputVec);
//        job->threadsPool = new pthread_t[multiThreadLevel];
//        job->jobState = {UNDEFINED_STAGE, 0};
//        job->barrierShuffle = new Barrier(multiThreadLevel);
//    }
//    catch (std::bad_alloc &e)
//    {
//        std::cerr << ERROR_INITIALIZED_JOB_CONTEXT;
//        exit(EXIT_FAILURE);
//    }
//
//    job->numThread = multiThreadLevel;
//    job->threadsContext = std::vector<threadJobContext>(multiThreadLevel);
//    for (int i = 0; i < multiThreadLevel; i++)
//    {
//        job->threadsContext[i].jobContext = job;
//    }
//
//    job->jobState = {MAP_STAGE, 0};
//    for (int i = 0; i < multiThreadLevel - 1; i++)
//    {
//        pt_res = pthread_create(&job->threadsPool[i], NULL, &mapPhase,
//                                (void *) &job->threadsContext[i]);
//        if (pt_res)
//        {
//            std::cerr << PTHREAD_CREATE_FAILED;
//            exit(EXIT_FAILURE);
//        }
//    }
//    int pt_shuffle = 0;
//    pt_shuffle = pthread_create(&job->threadsPool[multiThreadLevel - 1], NULL,
//                                &shufflePhase,
//                                (void *) &job->threadsContext[
//                                        multiThreadLevel - 1]);
//    if (pt_shuffle)
//    {
//        std::cerr << PTHREAD_CREATE_FAILED;
//        exit(EXIT_FAILURE);
//    }
//    return (void *) job;
//}
//
///**
// * This function produces a (K3*,V3*) pair for output vector
// * @param key - key
// * @param value - value
// * @param context - JobContext
// */
//void emit3(K3 *key, V3 *value, void *context)
//{
//    threadJobContext *threadContext = (threadJobContext *) context;
//    int res = pthread_mutex_lock(&threadContext->jobContext->pMutex);
//    if (res)
//    {
//        std::cerr << PTHREAD_LOCK_MUTEX;
//        exit(EXIT_FAILURE);
//    }
//    OutputPair pair = OutputPair(key, value);
//    threadContext->jobContext->outputVec.push_back(pair);
//    res = pthread_mutex_unlock(&threadContext->jobContext->pMutex);
//    if (res)
//    {
//        std::cerr << PTHREAD_UNLOCK_MUTEX;
//        exit(EXIT_FAILURE);
//    }
//}
//
///**
// * function gets the job handle returned by startMapReduceFramework and waits until it is finished.
// * @param job - JobHandle
// */
//void waitForJob(JobHandle job)
//{
//    JobContext *jobContext = (JobContext *) job;
//    try
//    {
//        for (int i = 0; i < jobContext->numThread; i++)
//        {
//            pthread_join(jobContext->threadsPool[i], NULL);
//        }
//    }
//    catch (...)
//    {
//        std::cerr << PTHREAD_JOIN_FAIL;
//    }
//}
//
///**
// * This function produces a (K2*,V2*) pair for intermidate vector
// * @param key
// * @param value
// * @param context
// */
//void emit2(K2 *key, V2 *value, void *context)
//{
//    threadJobContext *threadJob = (threadJobContext *) context;
//    int res = pthread_mutex_lock(&threadJob->mutex);
//    if (res)
//    {
//        std::cerr << PTHREAD_LOCK_MUTEX;
//        exit(EXIT_FAILURE);
//    }
//    IntermediatePair pair = IntermediatePair(key, value);
//    threadJob->threadVecIntermediate.push_back(pair);
//    res = pthread_mutex_unlock(&threadJob->mutex);
//    if (res)
//    {
//        std::cerr << PTHREAD_UNLOCK_MUTEX;
//        exit(EXIT_FAILURE);
//    }
//}
//
///**
// * this function gets a job handle and updates the state of the job into the given
//    JobState struct.
// * @param job - jobHandle
// * @param state  - current state
// */
//void getJobState(JobHandle job, JobState *state)
//{
//    JobContext *jobContext = (JobContext *) job;
//
//    int res = pthread_mutex_lock(&jobContext->pMutex);
//    if (res)
//    {
//        std::cerr << PTHREAD_LOCK_MUTEX;
//        exit(EXIT_FAILURE);
//    }
//    *state = JobState{jobContext->jobState.stage, jobContext->jobState.percentage};
//    res = pthread_mutex_unlock(&jobContext->pMutex);
//
//    if (res)
//    {
//        std::cerr << PTHREAD_UNLOCK_MUTEX;
//        exit(EXIT_FAILURE);
//    }
//}
//
///**
// * closeJobHandle – Releasing all resources of a job. You should prevent releasing resources
//  before the job is finished. After this function is called the job handle will be invalid.
// * @param job  - JobHandle
// */
//void closeJobHandle(JobHandle job)
//{
//    waitForJob(job);
//    JobContext *jobContext = (JobContext *) job;
//    delete jobContext;
//    return;;
//}