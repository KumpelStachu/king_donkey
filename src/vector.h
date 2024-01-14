#pragma once

typedef int (*comparator_t)(const void *, const void *);

#define VECTOR_DECL(name)                                         \
    typedef struct name##_vector                                  \
    {                                                             \
        name *data;                                               \
        size_t size;                                              \
        size_t capacity;                                          \
    } name##_vector;                                              \
    typedef int (*name##_comparator)(const name *, const name *); \
    name##_vector *name##_vector_new();                           \
    void name##_vector_free(name##_vector *v);                    \
    name *name##_vector_push(name##_vector *v, name *value);      \
    void name##_vector_pop(name##_vector *v);                     \
    name *name##_vector_at(name##_vector *v, size_t index);       \
    name *name##_vector_front(name##_vector *v);                  \
    name *name##_vector_back(name##_vector *v);                   \
    void name##_vector_erase(name##_vector *v, size_t i);         \
    void name##_vector_clear(name##_vector *v);                   \
    void name##_vector_reserve(name##_vector *v, size_t n);       \
    void name##_vector_resize(name##_vector *v, size_t n);        \
    void name##_vector_shrink(name##_vector *v);                  \
    void name##_vector_sort(name##_vector *v, name##_comparator cmp);

#define VECTOR_IMPL(name)                                               \
    name##_vector *name##_vector_new()                                  \
    {                                                                   \
        name##_vector *v = malloc(sizeof(*v));                          \
        assert(v != NULL);                                              \
        memset(v, 0, sizeof(*v));                                       \
        return v;                                                       \
    }                                                                   \
    void name##_vector_free(name##_vector *v)                           \
    {                                                                   \
        free(v->data);                                                  \
        free(v);                                                        \
    }                                                                   \
    name *name##_vector_push(name##_vector *v, name *value)             \
    {                                                                   \
        if (v->size == v->capacity)                                     \
        {                                                               \
            v->capacity = v->capacity == 0 ? 1 : v->capacity * 2;       \
            v->data = realloc(v->data, sizeof(*v->data) * v->capacity); \
            assert(v->data != NULL);                                    \
        }                                                               \
        v->data[v->size++] = *value;                                    \
        return &v->data[v->size - 1];                                   \
    }                                                                   \
    void name##_vector_pop(name##_vector *v)                            \
    {                                                                   \
        assert(v->size > 0);                                            \
        v->size--;                                                      \
    }                                                                   \
    name *name##_vector_at(name##_vector *v, size_t index)              \
    {                                                                   \
        assert(index < v->size);                                        \
        return &v->data[index];                                         \
    }                                                                   \
    name *name##_vector_front(name##_vector *v)                         \
    {                                                                   \
        assert(v->size > 0);                                            \
        return &v->data[0];                                             \
    }                                                                   \
    name *name##_vector_back(name##_vector *v)                          \
    {                                                                   \
        assert(v->size > 0);                                            \
        return &v->data[v->size - 1];                                   \
    }                                                                   \
    void name##_vector_erase(name##_vector *v, size_t i)                \
    {                                                                   \
        assert(i < v->size);                                            \
        memmove(&v->data[i], &v->data[i + 1],                           \
                sizeof(*v->data) * (v->size - i - 1));                  \
        v->size--;                                                      \
    }                                                                   \
    void name##_vector_clear(name##_vector *v)                          \
    {                                                                   \
        v->size = 0;                                                    \
    }                                                                   \
    void name##_vector_reserve(name##_vector *v, size_t n)              \
    {                                                                   \
        if (v->capacity < n)                                            \
        {                                                               \
            v->capacity = n;                                            \
            v->data = realloc(v->data, sizeof(*v->data) * v->capacity); \
            assert(v->data != NULL);                                    \
        }                                                               \
    }                                                                   \
    void name##_vector_resize(name##_vector *v, size_t n)               \
    {                                                                   \
        if (v->capacity < n)                                            \
        {                                                               \
            v->capacity = n;                                            \
            v->data = realloc(v->data, sizeof(*v->data) * v->capacity); \
            assert(v->data != NULL);                                    \
        }                                                               \
        v->size = n;                                                    \
    }                                                                   \
    void name##_vector_shrink(name##_vector *v)                         \
    {                                                                   \
        v->capacity = v->size;                                          \
        v->data = realloc(v->data, sizeof(*v->data) * v->capacity);     \
        assert(v->data != NULL);                                        \
    }                                                                   \
    void name##_vector_sort(name##_vector *v, name##_comparator cmp)    \
    {                                                                   \
        qsort(v->data, v->size, sizeof(*v->data), (comparator_t)cmp);   \
    }