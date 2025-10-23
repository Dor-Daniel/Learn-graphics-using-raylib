// QuadTree.hpp
#pragma once
#include "raylib.h"
#include <vector>
#include <algorithm>

static const int max_depth = 10;

template <typename T>
class QuadTree
{
private:
    bool debug_ = true;                 // off by default for perf
    int capacity_;
    Vector2 center_;
    float width_, height_;
    int depth_;

    bool hasChildren_;
    std::vector<QuadTree<T> *> children_; // owning raw ptrs
    std::vector<const T*> elements_;      // store pointers to external elements

    static bool contains(const Vector2 &c, float w, float h, const T &p)
    {
        const float left   = c.x - w * 0.5f;
        const float right  = c.x + w * 0.5f;
        const float top    = c.y - h * 0.5f;
        const float bottom = c.y + h * 0.5f;
        return (p.x >= left && p.x <= right && p.y >= top && p.y <= bottom);
    }

    static bool rectIntersectsNode(const Rectangle& r, const Vector2& c, float w, float h)
    {
        const Rectangle node{
            c.x - w * 0.5f,
            c.y - h * 0.5f,
            w, h
        };
        return !(r.x > node.x + node.width  ||
                 r.x + r.width < node.x     ||
                 r.y > node.y + node.height ||
                 r.y + r.height < node.y);
    }

    void collectElements(std::vector<const T*> &out) const
    {
        out.insert(out.end(), elements_.begin(), elements_.end());
        if (hasChildren_)
        {
            for (auto *ch : children_) ch->collectElements(out);
        }
    }

    void subdivide()
    {
        if (hasChildren_) return;

        const float hw = width_  * 0.5f;
        const float hh = height_ * 0.5f;

        const Vector2 cTL{center_.x - hw * 0.5f, center_.y - hh * 0.5f};
        const Vector2 cTR{center_.x + hw * 0.5f, center_.y - hh * 0.5f};
        const Vector2 cBL{center_.x - hw * 0.5f, center_.y + hh * 0.5f};
        const Vector2 cBR{center_.x + hw * 0.5f, center_.y + hh * 0.5f};

        children_.reserve(4);
        children_.push_back(new QuadTree<T>(cTL, hw, hh, capacity_, depth_ + 1));
        children_.push_back(new QuadTree<T>(cTR, hw, hh, capacity_, depth_ + 1));
        children_.push_back(new QuadTree<T>(cBL, hw, hh, capacity_, depth_ + 1));
        children_.push_back(new QuadTree<T>(cBR, hw, hh, capacity_, depth_ + 1));

        hasChildren_ = true;

        // Push current elements down
        for (const auto* e : elements_)
        {
            for (auto *ch : children_)
                if (contains(ch->center_, ch->width_, ch->height_, *e))
                    ch->insert(e);
        }
        elements_.clear();
    }

    void clearChildren()
    {
        if (!hasChildren_) return;
        for (auto *ch : children_) delete ch;
        children_.clear();
        hasChildren_ = false;
    }

public:
    QuadTree(const Vector2 &center, float width, float height, int capacity, int depth)
        : capacity_(capacity), center_(center), width_(width), height_(height),
          depth_(depth), hasChildren_(false) {}

    ~QuadTree() { clearChildren(); }

    void insert(const T* element)
    {
        if (hasChildren_)
        {
            for (auto *ch : children_)
                if (contains(ch->center_, ch->width_, ch->height_, *element))
                    ch->insert(element);
            return;
        }

        // If capacity exceeded & we can subdivide, do so once we insert
        if (!hasChildren_ && depth_ < max_depth && elements_.size() >= (size_t)capacity_)
        {
            elements_.push_back(element);
            subdivide();
            return;
        }

        elements_.push_back(element);
    }

    void setDebugMode(bool d) { debug_ = d; }

    // Rectangle query (broadphase); returns pointers potentially overlapping region.
    void rectQuery(const Rectangle &region, std::vector<const T *> &found) const
    {
        if (!rectIntersectsNode(region, center_, width_, height_)) return;

        if (!hasChildren_)
        {
            // Add all elements in this leaf
            found.insert(found.end(), elements_.begin(), elements_.end());
            return;
        }

        for (const auto *ch : children_)
            ch->rectQuery(region, found);
    }

    void drawDebug() const
    {
        if (!debug_) return;
        DrawRectangleLines(
            (center_.x - width_ * 0.5f),
            (center_.y - height_ * 0.5f),
            (width_),
            (height_),
            ORANGE);
        if (hasChildren_)
            for (auto *ch : children_) ch->drawDebug();
    }

    // Rebuild from an external authoritative set of items (recommended)
    void rebuild(const std::vector<T*> &items)
    {
        clearChildren();
        elements_.clear();
        hasChildren_ = false;
        elements_.reserve(items.size());
        for (const T* e : items) insert(e);
    }
};
