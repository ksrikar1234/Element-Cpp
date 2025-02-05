#ifndef __GP_TASKFLOW_HPP___
#ifndef __GP_TASKFLOW_HPP___

#include <iostream>
#include <fstream>

#include <string>
#include <vector>
#include <map>

#include <chrono>
#include <algorithm>
#include <stdexcept>
#include <initializer_list>

#include <functional>

#include <thread>
#include <atomic>
#include <future>

// Usage :
// gp_std::taskflowgraph graph;
// graph.set_executor(gp_std::async_executor::make());  

// graph.add_task("Task1", []
//                { std::cout << "Executing Task 1\n"; });
// graph.add_task("Task2", []
//                { std::cout << "Executing Task 2\n"; });
// graph.add_task("Task3", []
//                { std::cout << "Executing Task 3\n"; std::this_thread::sleep_for(std::chrono::seconds(2)); });
// graph.add_task("Task4", []
//                { std::cout << "Executing Task 4\n"; });
// graph.add_task("Task5", []
//                { std::cout << "Executing Task 5\n"; });

// graph.add_dependency("Task1", {"Task2", "Task3"});
// graph.add_dependency("Task2", "Task4");
// graph.add_dependency("Task3", "Task5");

// graph.execute();

// graph.export_to_graphviz("taskflow2.dot");

namespace gp_std
{
    class executor_base
    {
    public:
        virtual void enqueue(std::vector<std::function<void()>>& tasks) = 0;
        virtual ~executor_base() = default;
        virtual std::shared_ptr<executor_base> clone() const = 0;
    };

    class executor
    {
        public:
        executor(std::shared_ptr<executor_base> executor) : m_executor(executor) {}

        executor() : m_executor(nullptr) {}

        executor& operator=(std::shared_ptr<executor_base> executor)
        {
            m_executor = executor;
            return *this;
        }

        executor& operator=(const executor& other)
        {
            m_executor = other.m_executor;
            return *this;
        }
       
        void enqueue(std::vector<std::function<void()>> tasks)
        {
            if(m_executor) m_executor->enqueue(tasks);
        }

        std::shared_ptr<executor_base> clone() const
        {
            if(m_executor) return m_executor->clone();
            else throw std::runtime_error("executor is Not Set\n");
        }

        operator std::shared_ptr<executor_base> () const
        {
            if(m_executor) return m_executor;
            else throw std::runtime_error("executor is Not Set\n");
        }
  
        private:
        std::shared_ptr<executor_base> m_executor;
    };

    class sequential_executor : public executor_base
    {
    public:
        
        void enqueue(std::vector<std::function<void()>>& tasks) override
        {
            for (auto& task : tasks)
            {
                task();
            }
        }

        static std::shared_ptr<executor_base> make()  
        {
            return std::make_shared<sequential_executor>();
        }

        std::shared_ptr<executor_base> clone() const override
        {
            return std::make_shared<sequential_executor>(*this);
        }
        
        ~sequential_executor() override  = default;
    };

    // Async executor uses Futures
    class async_executor : public executor_base
    {
    public:
        void enqueue(std::vector<std::function<void()>>& tasks) override
        {
            std::vector<std::future<void>> futures;
        
            for (auto &task : tasks)
            {
                futures.emplace_back(std::async(std::launch::async, task));
            }

            for (auto &future : futures)
            {
                future.get();
            }
        }

        static std::shared_ptr<executor_base> make()
        {
            return std::make_shared<async_executor>();
        }

        std::shared_ptr<executor_base> clone() const override
        {
            return std::make_shared<async_executor>(*this);
        }

        ~async_executor() override = default;
    };


    class Timer
    {
    public:
        Timer() : m_start_time(std::chrono::steady_clock::now()) {}

        double now() const
        {
            return std::chrono::duration<double>(std::chrono::steady_clock::now() - m_start_time).count();
        }

        void reset()
        {
            m_start_time = std::chrono::steady_clock::now();
        }

    private:
        std::chrono::steady_clock::time_point m_start_time;
    };

    template <typename T>
    class Stable_VectorIdxPtr
    {
    public:
        Stable_VectorIdxPtr() : m_vec(nullptr), m_idx(0) {}
        Stable_VectorIdxPtr(std::nullptr_t) : m_vec(nullptr), m_idx(0) {}

        Stable_VectorIdxPtr(std::vector<T> &vec, size_t idx) : m_vec(&vec), m_idx(idx)
        {
            if (m_idx >= m_vec->size())
            {
                throw std::out_of_range("Index out of range");
            }

            if (m_vec->empty())
            {
                throw std::out_of_range("Vector is empty");
            }
        }

        Stable_VectorIdxPtr(const Stable_VectorIdxPtr &other) : m_vec(other.m_vec), m_idx(other.m_idx) {}
        Stable_VectorIdxPtr(Stable_VectorIdxPtr &&other) : m_vec(other.m_vec), m_idx(other.m_idx) {}

        Stable_VectorIdxPtr &operator=(const Stable_VectorIdxPtr &other)
        {
            m_vec = other.m_vec;
            m_idx = other.m_idx;
            return *this;
        }

        Stable_VectorIdxPtr &operator=(Stable_VectorIdxPtr &&other)
        {
            m_vec = other.m_vec;
            m_idx = other.m_idx;
            return *this;
        }

        T *operator->() { return &(vec()->at(m_idx)); }
        const T *operator->() const { return &(vec()->at(m_idx)); }

        T &operator*() { return vec()->at(m_idx); }
        const T &operator*() const { return vec()->at(m_idx); }

        bool operator==(const Stable_VectorIdxPtr &other) const { return m_vec == other.m_vec && m_idx == other.m_idx; }
        bool operator!=(const Stable_VectorIdxPtr &other) const { return !(*this == other); }

        bool operator==(T *other) const { return &(vec()->at(m_idx)) == other; }
        bool operator!=(T *other) const { return !(this == other); }

        bool operator==(std::nullptr_t) const { return m_vec == nullptr; }
        bool operator!=(std::nullptr_t) const { return m_vec != nullptr; }

        operator T *() { return &(vec()->at(m_idx)); }
        operator const T *() const { return &(vec()->at(m_idx)); }
        operator bool() const { return vec() != nullptr; }

        size_t index() const { return m_idx; }

    private:
        std::vector<T> *vec() const
        {
            if (m_vec == nullptr)
            {
                throw std::runtime_error("Null Pointer Acess In StableVectorIDXptr\n");
            }
            return m_vec;
        }

    private:
        std::vector<T> *m_vec;
        size_t m_idx;
    };

    class Task
    {
    public:
        Task(const char *name, std::function<void()> func) : m_name(std::move(name)), m_func(std::move(func)), m_execution_status(false) {}

        Task(const Task &other) : m_name(other.m_name), m_func(other.m_func), m_execution_status(other.m_execution_status.load()) {}
        Task(Task &&other) : m_name(std::move(other.m_name)), m_func(std::move(other.m_func)), m_execution_status(other.m_execution_status.load()) {}

        void add_dependency(Stable_VectorIdxPtr<Task> &task)
        {
            if (task == this)
                return;
            if (task == nullptr)
                return;

            if (task->find_cyclic_dependency(this) != nullptr)
            {
                printf("Circular Dependency Detected between %s and %s\n", m_name.c_str(), task->name().c_str());
                return;
            }

            m_dependencies.emplace_back(task);
        }

        Stable_VectorIdxPtr<Task> find_dependency(const char *name) const
        {
            for (auto &dep : m_dependencies)
            {
                if (dep->name() == name)
                {
                    return dep;
                }
            }
            return nullptr;
        }

        Stable_VectorIdxPtr<Task> find_dependency(const Stable_VectorIdxPtr<Task> &task) const
        {
            auto it = std::find(m_dependencies.begin(), m_dependencies.end(), task);
            if (it != m_dependencies.end())
            {
                return *it;
            }
            return nullptr;
        }

        Stable_VectorIdxPtr<Task> find_cyclic_dependency(const Task *task) const
        {
            for (auto &dep : m_dependencies)
            {
                if (dep == task)
                {
                    return dep;
                }
                else
                {
                    auto cyclic_dep = dep->find_cyclic_dependency(task);
                    if (cyclic_dep != nullptr)
                    {
                        return cyclic_dep;
                    }
                }
            }
            return nullptr;
        }

        void remove_dependency(const Stable_VectorIdxPtr<Task> &task)
        {
            m_dependencies.erase(std::remove(m_dependencies.begin(), m_dependencies.end(), task), m_dependencies.end());
        }

        std::vector<Stable_VectorIdxPtr<Task>> &get_dependencies() { return m_dependencies; }
        const std::vector<Stable_VectorIdxPtr<Task>> &get_dependencies() const { return m_dependencies; }

        bool ready() const
        {
            if (m_dependencies.empty())
            {
                return true;
            }

            for (auto &dep : m_dependencies)
            {
                if (!dep->is_executed())
                {
                    return false;
                }
            }

            return true;
        }

        const std::string &name() const { return m_name; }

        size_t get_dependency_count() const { return m_dependencies.size(); }

        bool is_executed() const { return m_execution_status.load(); }
        uint32_t execution_rank() const { return m_execution_rank; }

        bool operator<(const Task &other) const
        {
            return m_name < other.m_name;
        }

        bool operator==(const Task &other) const
        {
            return m_name == other.m_name;
        }

        bool operator<(const std::string &other) const
        {
            return m_name < other;
        }

        bool operator==(const std::string &other) const
        {
            return m_name == other;
        }

        bool operator==(const char *other) const
        {
            return m_name.compare(other) == 0;
        }

        void set_function(std::function<void()> func) { m_func = std::move(func); }

    private:
        friend class taskflowgraph;
        bool execute(std::vector<std::string> &exceptions, std::map<std::string, std::pair<double, double>> &execution_times, const Timer &timer, std::atomic<uint32_t> &executed_task_rank)
        {
            if (m_execution_status.load())
            {
                return true;
            }

            // Wait For Dependencies and yield if taking too long
            uint32_t spins = 0;
            while(!ready())
            {
                if(spins > 100000)
                {
                    std::this_thread::yield();
                    spins = 0;
                }
                ++spins;
            }
            
            double start_time = timer.now();

            try
            {
                m_func();
            }
            catch (const std::exception &e)
            {
                exceptions.emplace_back((m_name + " threw exception: ") + e.what());
                execution_times.erase(m_name);
                m_error_status.store(true);
                return false;
            }
            catch (...)
            {
                exceptions.emplace_back(m_name + " threw unknown exception.");
                execution_times.erase(m_name);
                m_error_status.store(true);
                return false;
            }

            double end_time = timer.now();

            m_execution_status.store(true);
           
            m_error_status.store(false);

            // Rank the executed task on the basis of completion order
            m_execution_rank = executed_task_rank.fetch_add(1);
            
            execution_times[m_name] = {start_time, end_time};

            return true;
        }

        bool has_error() const { return m_error_status.load(); }

    private:
        std::string m_name;
        std::function<void()> m_func;
        mutable std::atomic<bool> m_execution_status;
        std::atomic<bool> m_error_status;
        uint32_t m_execution_rank;
        std::vector<Stable_VectorIdxPtr<Task>> m_dependencies;
    };


    class taskflowgraph
    {
    public:
        taskflowgraph() : m_tasks(), m_tasks_map(), m_executor(async_executor::make()), m_timer(), m_execution_times(), m_exceptions() {}
        
        void set_executor(executor executor)
        {
            m_executor = executor;
        }

        void add_task(const char *name, std::function<void()> func)
        {
            auto it = (find_task(name));
            if (it != nullptr)
            {
                if((*it) != nullptr) {
                  (*it)->set_function(func);
                  return;
                }
            }
            m_tasks.emplace_back(name, func);
            m_tasks_map.emplace(name, Stable_VectorIdxPtr<Task>(m_tasks, m_tasks.size() - 1));
        }

        Stable_VectorIdxPtr<Task>* find_task(const char *name)
        {
            std::map<std::string, Stable_VectorIdxPtr<Task>>::iterator it = m_tasks_map.find(name);

            if (it != m_tasks_map.end())
            {
                return &(it->second);
            }

            return nullptr;
        }

        Task* task(const char *name)
        {
            if (find_task(name) == nullptr)
            {
                std::string error = std::string("Task not found: ") + name;
                throw std::runtime_error(error.c_str());
            }

            return (*find_task(name));
        }

        void add_dependency(const char *dependent_name, std::initializer_list<const char *> in_dependencies)
        {
            for (auto &dep : in_dependencies)
            {
                add_dependency(dependent_name, dep);
            }
        }

        void add_dependency(const char *dependent_name, const char *dependency_name)
        {
            Stable_VectorIdxPtr<Task>* task       = find_task(dependent_name);
            Stable_VectorIdxPtr<Task>* dependency = find_task(dependency_name);

            if (task == nullptr || dependency == nullptr)
            {
                return;
            }

            (*task)->add_dependency((*dependency));
        }

        void remove_dependency(const char *dependent_name, const char *dependency_name)
        {
            Stable_VectorIdxPtr<Task>* task       = find_task(dependent_name);
            Stable_VectorIdxPtr<Task>* dependency = find_task(dependency_name);

            if (task == nullptr || dependency == nullptr)
            {
                return;
            }

            (*task)->remove_dependency((*dependency));
        }

        void execute()
        {
            std::atomic<uint32_t> executed_tasks_rank(0);

            m_execution_times.clear();
            
            std::vector<std::function<void()>> curr_batch;

            while (all_tasks_executed() == false)
            {
                for (Task &task : m_tasks)
                {
                    if(task.has_error())
                    {
                        print_exceptions();
                        return;
                    }

                    if (!task.is_executed())
                    {
                        auto& rank  = executed_tasks_rank;
                        curr_batch.emplace_back([this, &task, &rank]()
                        {
                            task.execute(m_exceptions, m_execution_times, m_timer, rank);
                        });
                    }
                }

                m_executor.enqueue(curr_batch);
                curr_batch.clear();
            }

            printf("All Tasks Executed\n");
        }

        //+-------------------------------------------------------------------------------------------------+
        // Performance tracking, Visualization, Error handling & Debugging
        //+-------------------------------------------------------------------------------------------------+

        // Export to Graphviz
        void export_to_graphviz(const char *filename)
        {
            std::ofstream file(filename);
            file << "digraph taskflowgraph {\n";

            for (Task &task : m_tasks)
            {
                file << "  " << task.name() << " [label=\"" << task.name() << "\"];\n";

                // Add execution time
                std::map<std::string, std::pair<double, double>>::iterator it = m_execution_times.find(task.name());

                if (it != m_execution_times.end())
                {
                    // Add Execution Time
                    file << "  " << task.name() << " [label=\"" << task.name() << "\\n Rank-" << task.execution_rank() << "---> Time : " << it->second.first << "s - " << it->second.second << "s\"];\n";
                }
                else
                {
                    file << "  " << task.name() << " [label=\"" << task.name() << "\\n"
                         << "Not Executed\"];\n";
                }

                for (Stable_VectorIdxPtr<Task> &dep : task.get_dependencies())
                {
                    file << "  " << dep->name() << " -> " << task.name() << ";\n";
                }
            }

            file << "}\n";
            file.close();

            system("dot -Tpng taskflow2.dot -o taskflow2.png");
        }

        // Error handling & Debugging
        bool has_exceptions() const { return !(m_exceptions.empty()); }

        const std::vector<std::string> &exceptions() const { return m_exceptions; }

        void print_exceptions() const
        {
            for (auto &exception : m_exceptions)
            {
                std::cout << exception << std::endl;
            }
        }

        bool all_tasks_executed() const
        {
            for (auto &task : m_tasks)
            {
                if (!task.is_executed())
                {
                    return false;
                }
            }
            return true;
        }

        ~taskflowgraph()
        {
            
        }

    private:
        std::vector<Task> m_tasks;
        std::map<std::string, Stable_VectorIdxPtr<Task>> m_tasks_map;

        // executor
        executor m_executor;

        // Performance tracking
        Timer m_timer;
        std::map<std::string, std::pair<double, double>> m_execution_times;

        // Error handling
        std::vector<std::string> m_exceptions;
    };
} // namespace gp_std
#endif
