#!/usr/bin/env python3
"""Minimal, fast S-expression reader/writer for KiCad files."""

def parse(text):
    i = 0
    n = len(text)

    def skip_ws(i):
        while i < n and text[i] in " \t\r\n":
            i += 1
        return i

    def read_atom(i):
        start = i
        while i < n and text[i] not in " \t\r\n()":
            i += 1
        return text[start:i], i

    def read_string(i):
        start = i
        i += 1
        while i < n:
            if text[i] == '\\':
                i += 2
                continue
            if text[i] == '"':
                i += 1
                break
            i += 1
        return text[start:i], i

    def read_expr(i):
        i = skip_ws(i)
        if text[i] != '(':
            raise ValueError(f"expected ( at {i}: ...{text[max(0,i-20):i+20]}...")

        i += 1
        node = []

        while True:
            i = skip_ws(i)

            if i >= n:
                raise ValueError("unexpected EOF")
            if text[i] == ')':
                i += 1
                return node, i
            if text[i] == '(':
                sub, i = read_expr(i)
                node.append(sub)
            elif text[i] == '"':
                s, i = read_string(i)
                node.append(s)
            else:
                a, i = read_atom(i)
                node.append(a)

    i = skip_ws(0)
    top, i = read_expr(i)
    return top

def dumps(node):
    if isinstance(node, list):
        return "(" + " ".join(dumps(x) for x in node) + ")"
    return node

def _is_simple(node):
    # a list is "simple" (inline-able) if none of its children are lists
    return isinstance(node, list) and all(not isinstance(c, list) for c in node)

def write(node, out, depth=0):
    if not isinstance(node, list):
        out.append(node)
        return
    if _is_simple(node):
        out.append("(" + " ".join(node) + ")")
        return

    tab = "\t" * depth
    out.append("(")

    for i, item in enumerate(node):
        if i == 0:
            out.append(item)
            continue
        if isinstance(item, list):
            out.append("\n" + tab + "\t")
            write(item, out, depth + 1)
        else:
            out.append(" " + item)

    out.append("\n" + tab + ")")

def dump_file(tree, path):
    out = []
    write(tree, out, 0)
    with open(path, "w") as f:
        f.write("".join(out))
        f.write("\n")

def fmt_num(x):
    if isinstance(x, int):
        return str(x)

    s = f"{x:.6f}".rstrip("0").rstrip(".")
    if s in ("", "-0"):
        s = "0"

    return s

def find_all(node, key):
    """Yield all list nodes whose first element == key, recursively."""
    if isinstance(node, list):
        if node and node[0] == key:
            yield node

        for child in node:
            yield from find_all(child, key)

def get(node, key):
    """First child list starting with key, or None."""
    if isinstance(node, list):
        for child in node:
            if isinstance(child, list) and child and child[0] == key:
                return child

    return None

def get_all(node, key):
    if isinstance(node, list):
        return [c for c in node if isinstance(c, list) and c and c[0] == key]

    return []
