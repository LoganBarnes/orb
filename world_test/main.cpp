#include <iostream>
#include <vector>
#include <chrono>

using deleter_t = std::function<void(void *)>;
using unique_void_ptr = std::unique_ptr<void, deleter_t>;
using update_fun = std::function<void(double, double)>;

template<typename T>
auto deleter(void const *data) -> void
{
    auto const *p = static_cast<T const *>(data);
    std::cout << "{" << *p << "} located at [" << p << "] is being deleted.\n";
    delete p;
}

template<typename T, typename... Args>
auto make_unique_void(Args...args) -> unique_void_ptr
{
    auto *p = new T(std::forward<Args>(args)...);
    std::cout << "{" << *p << "} located at [" << p << "] is being created.\n";
    return unique_void_ptr(p, &deleter<T>);
}

template<typename, typename T>
struct has_update
{
    static_assert(std::integral_constant<T, false>::value, "Second template parameter needs to be of function type.");
};

// specialization that does the checking

template<typename C, typename Ret, typename... Args>
struct has_update<C, Ret(Args...)>
{
private:
    template<typename T>
    static constexpr auto check(T *)
    -> typename std::is_same<decltype(std::declval<T>().update(std::declval<Args>()...)), Ret>::type
    {
        throw std::runtime_error("Not to be used at runtime");
    }

    template<typename>
    static constexpr std::false_type check(...) { return std::integral_constant<bool, false>(); }

    typedef decltype(check<C>(0)) type;

public:
    static constexpr bool value = type::value;
};

template<typename TimeT = std::chrono::milliseconds>
struct measure
{
    template<typename F, typename ...Args>
    static auto duration(F &&func, Args &&... args)
    {
        auto start = std::chrono::steady_clock::now();
        std::forward<decltype(func)>(func)(std::forward<Args>(args)...);
        return std::chrono::duration_cast<TimeT>(std::chrono::steady_clock::now() - start);
    }
};


struct UpdateEntity
{
    virtual ~UpdateEntity() = default;
    virtual void update(double, double) = 0;
};

struct VirtualEntity : public UpdateEntity
{
    double pos[3]{};
    double vel[3]{};
    double acc[3]{};

    VirtualEntity() { std::cout << "VirtualEntity created" << std::endl; }
    ~VirtualEntity() override { std::cout << "VirtualEntity deleted" << std::endl; }

    void update(double t, double dt) override
    {
        acc[0] = t;
        vel[0] = acc[0] * dt;
        pos[0] = vel[0] * dt;
    }

    friend std::ostream &operator<<(std::ostream &os, const VirtualEntity &entity)
    {
        os << "Virtual (x=" << entity.pos[0] << ")";
        return os;
    }
};

struct StaticEntity
{
    double pos[3];
    double vel[3];
    double acc[3];

    void update(double t, double dt)
    {
        acc[0] = t;
        vel[0] = acc[0] * dt;
        pos[0] = vel[0] * dt;
    }

    friend std::ostream &operator<<(std::ostream &os, const StaticEntity &entity)
    {
        os << "Static (x=" << entity.pos[0] << ")";
        return os;
    }
};

class Entity
{
public:
    explicit Entity(unique_void_ptr upEntity, update_fun updateFun)
            : upEntity_{std::move(upEntity)}, updateFun_(std::move(updateFun)) {}

    void update(double t, double dt) { updateFun_(t, dt); }

private:
    unique_void_ptr upEntity_;
    update_fun updateFun_;
};

struct FriendEntity
{
    double pos[3]{};
    double vel[3]{};
    double acc[3]{};

    FriendEntity() { std::cout << "FriendEntity created" << std::endl; }
    ~FriendEntity() { std::cout << "FriendEntity deleted" << std::endl; }

    friend void update(FriendEntity &entity, double t, double dt)
    {
        entity.acc[0] = t;
        entity.vel[0] = entity.acc[0] * dt;
        entity.pos[0] = entity.vel[0] * dt;
    }

    friend std::ostream &operator<<(std::ostream &os, const FriendEntity &entity)
    {
        os << "Friend (x=" << entity.pos[0] << ")";
        return os;
    }
};


class World
{
public:
    explicit World(double dt = 0.1) : dt_{dt} {}

    template<typename T, typename...Args>
    void addEntity(Args &&...args)
    {
        static_assert(has_update<T, void(double, double)>::value,
                      "Entity classes must define: 'void update(double, double)'");
        auto upEntity = make_unique_void<T>(std::forward<Args>(args)...);
        auto *pEntity = static_cast<T *>(upEntity.get());
        entities_.emplace_back(std::move(upEntity), [pEntity](double t, double dt)
        {
            pEntity->update(t, dt);
        });
    }

    void loop(int iterations)
    {
        for (int i = 0; i < iterations; ++i)
        {
            for (auto &e : entities_)
            {
                e.update(t_, dt_);
            }
            t_ += dt_;
        }
    }
private:
    std::vector<Entity> entities_;
    double t_{0};
    double dt_;
};

class VirtualWorld
{
public:
    explicit VirtualWorld(double dt = 0.1) : dt_{dt} {}

    void addEntity(std::unique_ptr<UpdateEntity> upEntity)
    {
        entities_.emplace_back(std::move(upEntity));
    }

    void loop(int iterations)
    {
        for (int i = 0; i < iterations; ++i)
        {
            for (auto &e : entities_)
            {
                e->update(t_, dt_);
            }
            t_ += dt_;
        }
    }
private:
    std::vector<std::unique_ptr<UpdateEntity>> entities_;
    double t_{0};
    double dt_;
};


class FriendWorld
{
public:
    explicit FriendWorld(double dt = 0.1) : dt_{dt} {}

    void addEntity(std::unique_ptr<UpdateEntity> upEntity)
    {
        entities_.emplace_back(std::move(upEntity));
    }

    void loop(int iterations)
    {
        for (int i = 0; i < iterations; ++i)
        {
            for (auto &e : entities_)
            {
                e->update(t_, dt_);
            }
            t_ += dt_;
        }
    }
private:
    std::vector<std::unique_ptr<UpdateEntity>> entities_;
    double t_{0};
    double dt_;
};


struct SeanEntity
{
    template<typename T>
    explicit SeanEntity(T entity) : self_{std::make_unique<Entity<T>>(std::move(entity))} {}

    void update(double t, double dt)
    {
        self_->update(t, dt);
    }

private:
    struct UpdateEntity
    {
        virtual ~UpdateEntity() = default;
        virtual void update(double, double) = 0;
    };

    template<typename T>
    struct Entity : public UpdateEntity
    {
        explicit Entity(T entity) : data_{std::move(entity)} {}

        void update(double t, double dt) final
        {
            data_.update(t, dt);
        }
        T data_;
    };

    std::unique_ptr<UpdateEntity> self_;
};


class SeanWorld
{
public:
    explicit SeanWorld(double dt = 0.1) : dt_{dt} {}

    template<typename T>
    void addEntity(T entity)
    {
        entities_.emplace_back(std::move(entity));
    }

    void loop(int iterations)
    {
        for (int i = 0; i < iterations; ++i)
        {
            for (auto &e : entities_)
            {
                e.update(t_, dt_);
            }
            t_ += dt_;
        }
    }
private:
    std::vector<SeanEntity> entities_;
    double t_{0};
    double dt_;
};


int main()
{
    double dt = 0.001;
    int iterations = 1000000000;
    World world(dt);
    VirtualWorld vworld(dt);
    SeanWorld sworld(dt);

    world.addEntity<StaticEntity>();

    std::cout << std::endl;

    std::cout << "non-virtual: "
              << measure<>::duration([&]
                                     {
                                         world.loop(iterations);
                                     }).count()
              << "ms\n" << std::endl;


    auto upVe = std::make_unique<VirtualEntity>();
    vworld.addEntity(std::move(upVe));

    std::cout << "virtual: "
              << measure<>::duration([&]
                                     {
                                         vworld.loop(iterations);
                                     }).count()
              << "ms\n" << std::endl;

    sworld.addEntity(StaticEntity());

    std::cout << "sean: "
              << measure<>::duration([&]
                                     {
                                         sworld.loop(iterations);
                                     }).count()
              << "ms\n" << std::endl;

    return 0;
}