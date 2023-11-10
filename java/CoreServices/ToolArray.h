#ifndef TOOLARRAY_H
#define TOOLARRAY_H

template <typename T, int SIZE>
class ToolArray
{
public:
  T array[SIZE];
  // Los valores colocados aquí dependen del criterio de ordenamiento
  int32_t sortindex[SIZE];
  int32_t* psortindex[SIZE];
private:
  int size;
public:
  static int cmp(const void* v1, const void* v2)
  {
    const int i1 = **(const int**)v1;
    const int i2 = **(const int**)v2;

    return i1 < i2 ? -1 : (i1 > i2);
  }

  ToolArray()
  {
    size = 0;
    memset2(array);
    memset2(sortindex);
    memset2(psortindex);
  }

  T* getLast()
  {
    return &array[size];
  }

  void setSortIndex(int32_t sort)
  {
    sortindex[size] = sort;
  }

  void sort()
  {
    int i;

    for (i = 0; i < size; ++i)
      psortindex[i] = &sortindex[i];

    qsort(psortindex, size, sizeof(int*), cmp);
  }

  void setSize(int32_t s)
  {
    size = s;
  }

  int getSize()
  {
    return size;
  }

  void incSize()
  {
    ++size;
  }

  // Devuelve los elementos de acuerdo al ordenamiento
  T& operator[](int32_t idx)
  {
    return array[psortindex[idx] - sortindex];
  }
};

template <typename T, int SIZE>
class ToolDynamicArray
{
public:
  T* array;
  // Los valores colocados aquí dependen del criterio de ordenamiento
  int sortindex[SIZE];
  int* psortindex[SIZE];
private:
  int size;
public:
  static int cmp(const void* v1, const void* v2)
  {
    const int i1 = **(const int**)v1;
    const int i2 = **(const int**)v2;

    return i1 < i2 ? -1 : (i1 > i2);
  }

  ToolDynamicArray()
  {
    array = new T[SIZE];
    size = 0;
    memset2(sortindex);
    memset2(psortindex);
  }

  ~ToolDynamicArray()
  {
    release();
  }

  void release()
  {
    if (array != nullptr)
    {
      delete [] array;
      array = NULL;
    }
  }

  T* getLast() &
  {
    return &array[size];
  }

  void setSortIndex(int32_t sort)
  {
    sortindex[size] = sort;
  }

  void sort()
  {
    int i;

    for (i = 0; i < size; ++i)
      psortindex[i] = &sortindex[i];

    qsort(psortindex, size, sizeof(int*), cmp);
  }

  void setSize(int32_t s)
  {
    size = s;
  }

  int getSize()
  {
    return size;
  }

  void incSize()
  {
    ++size;
  }

  // Devuelve los elementos de acuerdo al ordenamiento
  T& operator[](int32_t idx)
  {
    return array[psortindex[idx] - sortindex];
  }

  T* get(int32_t idx)
  {
    return &array[psortindex[idx] - sortindex];
  }
};

#endif
