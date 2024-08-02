## createPlayer

```C 
static std::shared_ptr<PlayerBase> 
    createPlayer(const toolkit::EventPoller::Ptr &poller, const std::string &strUrl)
```

![image-20240801184254234](./PlayerBase.assets/image-20240801184254234.png)

## PlayerImp

```C++
template<typename Parent, typename Delegate>
class PlayerImp : public Parent {
    public:
    using Ptr = std::shared_ptr<PlayerImp>;

    template<typename ...ArgsType>
    PlayerImp(ArgsType &&...args) : Parent(std::forward<ArgsType>(args)...) {}

    void play(const std::string &url) override {
        return _delegate ? _delegate->play(url) : Parent::play(url);
    }
    ......
    protected:
    ......
    std::shared_ptr<Delegate> _delegate;
};
```

`Delegate`一般是`PlayerBase`，`Parent`一般是具体的`Player`类

对于类中的函数，有`_delegate`的情况下，优先使用`_delegate`的，其次再使用`Parent`的



## 类图——工厂模式

![QQ_1722502991723](./PlayerBase.assets/QQ_1722502991723.png)

ZLMediaKit中各个Player与PlayerBase之间的设计采用了工厂模式