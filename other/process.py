def proces2(l):
    return sorted((x * x for x in {*sum(l, [])}), reverse=1)

from numpy import array as a, sort as s
def proces3(l):
    return s(-a([*{*sum(l, [])}])) ** 2

if __name__ == '__main__':
    print(proces3([[1, 6], [3], [5], [5], [5]]))

# 1) уменьшение уровня вложенности sum(l, [])
# 2) удаление дубликатов {*previous}
# 3) квадрата + сортировка ??
