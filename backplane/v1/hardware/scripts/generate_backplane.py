#!/usr/bin/env python3
"""Generate the 2U/3U backplane schematic + PCB from the 1U design files.

Connector slots follow this format (bottom-up):
    - Battery tray
    - EPS
    - OBC
    - Comms
    - ADCS
    - `n` Generic slots
    - Payload

Known limitations:
    - SLOT_ID0-2 is a 3-bit code (8 values). The 3U's 12 slots exceed that, so slots
    beyond the 8th are intentionally left without real keying for now.
    - The new top-cap mounting holes are added to the PCB only, not mirrored
    as schematic symbols.

Usage:
  python3 generate_backplane.py             # regenerate all targets below
  python3 generate_backplane.py --target 2u # regenerate just one
"""

import argparse
import copy
import shutil
import sys
import uuid
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from sexp import parse, get, get_all, dump_file, fmt_num

HW_ROOT = Path(__file__).resolve().parent.parent
SRC_DIR = HW_ROOT / "1u"
SRC_SCH = SRC_DIR / "backplane.kicad_sch"
SRC_PCB = SRC_DIR / "backplane.kicad_pcb"
SUPPORT_FILES = [
    "backplane.kicad_pro",
    "backplane.kicad_prl",
    "fp-lib-table",
    "design-block-lib-table",
    "backplane.kicad_dru",  # carries the Allow_Edge_Plating_GND negative-clearance rule
]

STITCH_PITCH = 2.54        # mm, spacing of the GND stitching vias along the straight edges

SLOT_PITCH_Y = 18.0        # mm, PCB connector stacking pitch
GRID_DX = 161.29           # mm, schematic column spacing (matches J1->J3)
GRID_DY = 68.58            # mm, schematic row spacing (matches J1->J2)
TOP_CAP_BOUNDARY = 61.75   # mm, board Y at/below which Edge.Cuts geometry belongs to the "top cap"
BOARD_TOP_EDGE = 51.75     # mm, the board's actual top edge Y in the 1U source file
BOARD_LEFT_EDGE_X = 108.0  # mm, the board's left/right Edge.Cuts X in the 1U source file
BOARD_RIGHT_EDGE_X = 193.0

# new slots per target, beyond J1-J4
SLOT_MANIFESTS = {
    "2u": [("J5", "ADCS"), ("J6", "GENERIC"), ("J7", "GENERIC"), ("J8", "PAYLOAD")],
    "3u": [
        ("J5", "ADCS"), ("J6", "GENERIC"), ("J7", "GENERIC"), ("J8", "GENERIC"),
        ("J9", "GENERIC"), ("J10", "GENERIC"), ("J11", "GENERIC"), ("J12", "Payload"),
    ],
}

# ---------- helpers ----------

def new_uuid():
    return f'"{uuid.uuid4()}"'

def qstr(s):
    return f'"{s}"'

def unq(s):
    return s.strip('"')

def regen_uuids(node):
    """Recursively replace every (uuid "...") value with a fresh uuid, in place."""
    if isinstance(node, list):
        if len(node) >= 2 and node[0] == "uuid":
            node[1] = new_uuid()

        for child in node:
            regen_uuids(child)

def translate_points(node, dx, dy):
    """Recursively shift the numeric x,y of any (at|xy|start|end|mid|center x y [rot]) node."""
    if not isinstance(node, list):
        return

    if node and node[0] in ("at", "xy", "start", "end", "mid", "center") and len(node) >= 3:
        try:
            x = float(node[1]); y = float(node[2])
            node[1] = fmt_num(x + dx)
            node[2] = fmt_num(y + dy)
        except ValueError:
            pass

    for child in node:
        translate_points(child, dx, dy)

def set_property_text(node, prop_name, new_value):
    for p in get_all(node, "property"):
        if p[1] == qstr(prop_name):
            p[2] = qstr(new_value)
            return True

    return False

def get_property_text(node, prop_name):
    for p in get_all(node, "property"):
        if p[1] == qstr(prop_name):
            return unq(p[2])

    return None

def update_instances_reference(sym_node, new_ref):
    inst = get(sym_node, "instances")
    if inst is None:
        return

    for proj in get_all(inst, "project"):
        for path in get_all(proj, "path"):
            for ref_node in get_all(path, "reference"):
                ref_node[1] = qstr(new_ref)

# ---------- schematic ----------

def connector_positions(tree):
    out = {}
    for s in get_all(tree, "symbol"):
        lib_id = s[1][1] if isinstance(s[1], list) else None

        if lib_id and "Conn_02x20" in lib_id:
            ref = get_property_text(s, "Reference")
            out[ref] = s

    return out

def nearest_ref(pt, conn_pos):
    best, bd = None, float("inf")
    for ref, node in conn_pos.items():
        at = get(node, "at")
        cx, cy = float(at[1]), float(at[2])
        d = (pt[0] - cx) ** 2 + (pt[1] - cy) ** 2

        if d < bd:
            bd, best = d, ref

    return best

def build_j1_template(tree):
    """Return J1's slot as a template: its connector symbol plus every wire,
    global_label, junction and GND symbol nearest to it"""

    conns = connector_positions(tree)
    j1_node = conns["J1"]
    j1_at = get(j1_node, "at")
    j1_pos = (float(j1_at[1]), float(j1_at[2]))

    wires, labels, juncs, gnds = [], [], [], []
    for w in get_all(tree, "wire"):
        pts = get_all(get(w, "pts"), "xy")

        if pts and all(nearest_ref((float(p[1]), float(p[2])), conns) == "J1" for p in pts):
            wires.append(w)

    for gl in get_all(tree, "global_label"):
        at = get(gl, "at")

        if nearest_ref((float(at[1]), float(at[2])), conns) == "J1":
            labels.append(gl)

    for j in get_all(tree, "junction"):
        at = get(j, "at")

        if nearest_ref((float(at[1]), float(at[2])), conns) == "J1":
            juncs.append(j)

    for s in get_all(tree, "symbol"):
        lib_id = s[1][1] if isinstance(s[1], list) else None

        if lib_id == '"power:GND"':
            at = get(s, "at")

            if nearest_ref((float(at[1]), float(at[2])), conns) == "J1":
                gnds.append(s)

    label_text_node, best_d = None, float("inf")
    for t in get_all(tree, "text"):
        at = get(t, "at")
        if at is None or len(unq(t[1])) > 12:
            continue  # skip the stackup note, keep only short subsystem-name labels

        x, y = float(at[1]), float(at[2])
        d = (x - j1_pos[0]) ** 2 + (y - j1_pos[1]) ** 2

        if d < best_d:
            best_d, label_text_node = d, t

    return {"symbol": j1_node, "wires": wires, "labels": labels,
            "juncs": juncs, "gnds": gnds, "text": label_text_node}

def max_pwr_ref(tree):
    best = 0

    for s in get_all(tree, "symbol"):
        lib_id = s[1][1] if isinstance(s[1], list) else None

        if lib_id == '"power:GND"':
            ref = get_property_text(s, "Reference")

            if ref and ref.startswith("#PWR"):
                try:
                    best = max(best, int(ref[4:]))
                except ValueError:
                    pass

    return best

def clone_translated(node, dx, dy):
    clone = copy.deepcopy(node)

    translate_points(clone, dx, dy)
    regen_uuids(clone)

    return clone

def clone_connector(sym_node, dx, dy, new_ref):
    node = clone_translated(sym_node, dx, dy)

    set_property_text(node, "Reference", new_ref)
    update_instances_reference(node, new_ref)

    return node

def clone_gnd(gnd_template, dx, dy, new_ref):
    node = clone_translated(gnd_template, dx, dy)

    set_property_text(node, "Reference", new_ref)
    update_instances_reference(node, new_ref)

    return node

def clone_text(text_node, dx, dy, new_text):
    node = clone_translated(text_node, dx, dy)
    node[1] = qstr(new_text)

    return node

def fit_paper_size(tree, margin=30.0, grid=10.0):
    """Grow the sheet's paper size to fit everything placed on it.
    New slots are laid out in additional schematic grid columns, so
    a fixed A4 sheet overflows once enough are added."""
    xs, ys = [], []
    def walk(node):
        if not isinstance(node, list):
            return

        if node and node[0] == "lib_symbols":
            return

        if node and node[0] in ("at", "xy", "start", "end", "mid", "center") and len(node) >= 3:
            try:
                xs.append(float(node[1])); ys.append(float(node[2]))
            except ValueError:
                pass

        for c in node:
            walk(c)

    for top in tree:
        if isinstance(top, list) and top and top[0] == "lib_symbols":
            continue

        walk(top)

    import math
    width = math.ceil((max(xs) + margin) / grid) * grid
    height = math.ceil((max(ys) + margin) / grid) * grid

    paper = get(tree, "paper")
    if paper is not None:
        paper[:] = ["paper", qstr("User"), fmt_num(width), fmt_num(height)]

def extend_schematic(new_slots, out_path):
    """new_slots: list of (ref, name), placed in schematic grid order starting
    at grid index 4 (col=i//2, row=i%2), purely for schematic layout and
    unrelated to physical PCB stacking order."""
    tree = parse(SRC_SCH.read_text())
    tmpl = build_j1_template(tree)
    pwr_n = max_pwr_ref(tree)
    new_symbol_uuids = {}

    for i, (ref, name) in enumerate(new_slots, start=4):
        dx = (i // 2) * GRID_DX
        dy = (i % 2) * GRID_DY

        conn = clone_connector(tmpl["symbol"], dx, dy, ref)
        tree.append(conn)
        new_symbol_uuids[ref] = unq(get(conn, "uuid")[1])

        for w in tmpl["wires"]:
            tree.append(clone_translated(w, dx, dy))

        for gl in tmpl["labels"]:
            tree.append(clone_translated(gl, dx, dy))

        for j in tmpl["juncs"]:
            tree.append(clone_translated(j, dx, dy))

        for gnd in tmpl["gnds"]:
            pwr_n += 1
            tree.append(clone_gnd(gnd, dx, dy, f"#PWR{pwr_n:02d}"))

        if tmpl["text"] is not None:
            tree.append(clone_text(tmpl["text"], dx, dy, name))

    fit_paper_size(tree)
    dump_file(tree, out_path)

    return new_symbol_uuids

# ---------- pcb ----------

def footprint_ref(fp):
    for p in get_all(fp, "property"):
        if p[1] == '"Reference"':
            return unq(p[2])

    return None

def get_fp(tree, ref):
    for fp in get_all(tree, "footprint"):
        if footprint_ref(fp) == ref:
            return fp

    return None

def clone_footprint(fp, dx, dy, new_ref, new_path_uuid=None):
    node = copy.deepcopy(fp)
    at = get(node, "at")

    at[1] = fmt_num(float(at[1]) + dx)
    at[2] = fmt_num(float(at[2]) + dy)

    regen_uuids(node)

    for p in get_all(node, "property"):
        if p[1] == '"Reference"':
            p[2] = qstr(new_ref)

    if new_path_uuid is not None:
        path_node = get(node, "path")
        if path_node is not None:
            path_node[1] = qstr("/" + new_path_uuid)

    return node

def _shift_if_top_cap(node, delta_height):
    """Recursively shift the y of any (at|xy|start|end|mid|center x y [rot])
    descendant that's at/above TOP_CAP_BOUNDARY, by -delta_height. Skips
    filled_polygon subtrees since they are recomputed upon refilling."""
    if not isinstance(node, list):
        return

    if node and node[0] == "filled_polygon":
        return

    if node and node[0] in ("at", "xy", "start", "end", "mid", "center") and len(node) >= 3:
        try:
            y = float(node[2])
            if y <= TOP_CAP_BOUNDARY + 1e-6:
                node[2] = fmt_num(y - delta_height)
        except ValueError:
            pass

    for child in node:
        _shift_if_top_cap(child, delta_height)

def shift_top_cap(tree, delta_height, board_title):
    """Translate everything that belongs to the board's top cap up
    by delta_height: Edge.Cuts corner/notch geometry (stretching the long
    body side-edge lines to meet it), the F.Cu edge-plating gr_poly (an exact
    copy of the Edge.Cuts point list, so it tracks it identically), any
    stitching vias already up there, and each zone's own boundary polygon."""
    for top in tree:
        if not isinstance(top, list) or not top:
            continue

        if top[0] in ("footprint", "dimension"):
            continue

        _shift_if_top_cap(top, delta_height)

        if top[0] == "gr_text" and top[1] == '"OSUSAT BACKPLANE 1U\\nV1R1"':
            top[1] = qstr(f"OSUSAT BACKPLANE {board_title}\\nV1R1")

def _board_content_bbox(tree):
    """Bounding box of everything that has to clear the title block vertically: the
    Edge.Cuts outline AND every zone's own polygon/filled_polygon. The 1U
    source draws zone boundaries deliberately oversized relative to the board
    edge (so fills always fully cover it regardless of outline tweaks)"""
    xs, ys = [], []
    for kind in ("gr_line", "gr_arc", "gr_rect", "gr_circle", "gr_poly"):
        for node in get_all(tree, kind):
            layer = get(node, "layer")

            if not (layer and layer[1] == '"Edge.Cuts"'):
                continue

            for key in ("start", "end", "mid", "center"):
                p = get(node, key)
                if p:
                    xs.append(float(p[1])); ys.append(float(p[2]))

            pts = get(node, "pts")

            if pts:
                for xy in get_all(pts, "xy"):
                    xs.append(float(xy[1])); ys.append(float(xy[2]))

    for zone in get_all(tree, "zone"):
        for key in ("polygon", "filled_polygon"):
            for poly in get_all(zone, key):
                pts = get(poly, "pts")

                if pts:
                    for xy in get_all(pts, "xy"):
                        xs.append(float(xy[1])); ys.append(float(xy[2]))

    return min(xs), max(xs), min(ys), max(ys)

def _shift_pcb_rigidly(tree, dx, dy):
    """Translate every absolute-coordinate item on the board by (dx, dy), as
    one rigid body. Footprints are treated differently since most of thier properties are relative,
    so they must NOT also be shifted or the footprint's internal geometry would be corrupted."""
    if dx == 0 and dy == 0:
        return

    for top in tree:
        if not isinstance(top, list) or not top:
            continue

        if top[0] == "footprint":
            at = get(top, "at")

            if at:
                at[1] = fmt_num(float(at[1]) + dx)
                at[2] = fmt_num(float(at[2]) + dy)
        else:
            translate_points(top, dx, dy)

def center_and_fit_pcb(tree, margin=45.0, grid=10.0):
    """Recenter the whole board as a rigid body so it sits with equal margin on all sides,
    then size the PCB drawing-sheet paper to match. Needed because extending
    the board outline upward moves new content into negative-Y world space,
    which is off the top of a page anchored at world (0,0); growing the page
    size alone leaves the board off-center or off-page.

    margin is 45mm since KiCad's default worksheet
    draws a title block strip roughly 30-35mm tall along the bottom edge, plus
    a ~10mm border, and the board must clear that, not just the page edge."""
    import math
    min_x, max_x, min_y, max_y = _board_content_bbox(tree)

    width = math.ceil((max_x - min_x + 2 * margin) / grid) * grid
    height = math.ceil((max_y - min_y + 2 * margin) / grid) * grid

    # center exactly within the page, splitting the rounding
    # slack evenly instead of dumping it all on one side
    dx = (width - (max_x - min_x)) / 2 - min_x
    dy = (height - (max_y - min_y)) / 2 - min_y

    _shift_pcb_rigidly(tree, dx, dy)

    paper = get(tree, "paper")
    if paper is not None:
        paper[:] = ["paper", qstr("User"), fmt_num(width), fmt_num(height)]

def add_edge_stitching_vias(tree, delta_height):
    """Continue the existing left/right edge-plating stitching-via columns
    up through the newly stretched straight-edge section. 
    Stops before the relocated top cap's own via population to avoid overlap."""
    edge_columns = {}

    for v in get_all(tree, "via"):
        at = get(v, "at")
        net = get(v, "net")

        if not (net and net[1] == '"GND"'):
            continue

        x, y = float(at[1]), float(at[2])
        if TOP_CAP_BOUNDARY < y < 136.25 and (
            abs(x - BOARD_LEFT_EDGE_X) < 2.0 or abs(x - BOARD_RIGHT_EDGE_X) < 2.0
        ):
            edge_columns.setdefault(round(x, 2), []).append((y, v))

    new_cap_boundary = TOP_CAP_BOUNDARY - delta_height
    for entries in edge_columns.values():
        entries.sort(key=lambda e: e[0])
        topmost_y, template = entries[0]
        y = topmost_y - STITCH_PITCH

        while y > new_cap_boundary + 1e-6:
            clone = copy.deepcopy(template)
            get(clone, "at")[2] = fmt_num(y)
            regen_uuids(clone)
            tree.append(clone)
            y -= STITCH_PITCH

def add_side_mounting_holes(tree, delta_height, next_h):
    """Continue the M4 rail mounting-hole rows up
    through the newly stretched section at the same 27.25mm pitch, so
    structural support stays evenly spaced across the whole board instead of
    only existing at the two caps. Returns the next free H-number."""
    columns = {}
    for fp in get_all(tree, "footprint"):
        if fp[1] != '"MountingHole:MountingHole_4.3mm_M4"':
            continue

        at = get(fp, "at")
        x, y = float(at[1]), float(at[2])
        columns.setdefault(round(x, 1), []).append((y, fp))

    for entries in columns.values():
        entries.sort(key=lambda e: e[0])
        if len(entries) < 2:
            continue

        pitch = entries[1][0] - entries[0][0]
        topmost_y, template = entries[0]
        # the top-cap's own M4 hole (added separately, at a fixed offset from
        # the new topmost connector) lands at topmost_y - delta_height; stop
        # interpolating with at least half a pitch of clearance from it so
        # the two placement passes can never collide, whatever delta_height is
        cap_y = topmost_y - delta_height
        min_gap = pitch * 0.5
        y = topmost_y - pitch
        while y - min_gap > cap_y:
            clone = clone_footprint(template, 0.0, y - topmost_y, f"H{next_h}")
            tree.append(clone)
            next_h += 1
            y -= pitch
    return next_h

KEYING_X_BASE = 159.8   # mm, X of the first (J1) keying slot
KEYING_SLOT_W = 2.0     # mm
KEYING_X_STEP = 2.0     # mm, == slot width, so positions still pack edge-to-edge
KEYING_SLOT_H = 3.0
KEYING_Y_OFFSET = 2.25  # mm, fixed offset from connector center. this is set by
                        # the right-angle THT connector standing proud of the board
                        # surface, common to every board
KEYING_X_POSITIONS = 12  # 12 * 2.0mm = 24mm, fits before the M4 hole column at
                         # X=188 (keepout starts ~184.85mm) with ~1mm to spare

def add_keying_slots(tree, all_slots_with_y):
    """Rebuild the Layer-2 mechanical keying scheme for every slot
    on the board

    X position is assigned by physical Y order (stacking position), not by
    reference number

    Silkscreen must be added by hand."""
    template = None
    for node in get_all(tree, "gr_rect"):
        layer = get(node, "layer")

        if layer and layer[1] == '"Edge.Cuts"':
            template = copy.deepcopy(node)
            break

    if template is None:
        return

    for node in list(tree):
        if isinstance(node, list) and node and node[0] == "gr_rect":
            layer = get(node, "layer")

            if layer and layer[1] == '"Edge.Cuts"':
                tree.remove(node)

    for slot_number, (ref, conn_y) in enumerate(sorted(all_slots_with_y, key=lambda t: t[1])):
        if slot_number >= KEYING_X_POSITIONS:
            continue  # beyond this scheme's capacity

        x0 = KEYING_X_BASE + KEYING_X_STEP * slot_number
        y0 = conn_y + KEYING_Y_OFFSET

        clone = copy.deepcopy(template)

        get(clone, "start")[1:] = [fmt_num(x0), fmt_num(y0)]
        get(clone, "end")[1:] = [fmt_num(x0 + KEYING_SLOT_W), fmt_num(y0 + KEYING_SLOT_H)]

        regen_uuids(clone)

        tree.append(clone)

def extend_pcb(new_slots, out_path, new_symbol_uuids, board_title):
    tree = parse(SRC_PCB.read_text())
    j1 = get_fp(tree, "J1")
    j1_y = float(get(j1, "at")[2])
    margin = j1_y - BOARD_TOP_EDGE

    all_slot_ys = [(ref, float(get(get_fp(tree, ref), "at")[2])) for ref in ("J1", "J2", "J3", "J4")]

    n_new = len(new_slots)
    delta_height = 2 * margin + SLOT_PITCH_Y * (n_new - 1)

    for i, (ref, _name) in enumerate(new_slots):
        new_y = BOARD_TOP_EDGE - margin - SLOT_PITCH_Y * i
        clone = clone_footprint(j1, 0.0, new_y - j1_y, ref, new_symbol_uuids.get(ref))
        tree.append(clone)
        all_slot_ys.append((ref, new_y))

    # fill the M4 rail-mounting rows through the stretched section first,
    # while H1-H6 are still the only M4 footprints in the tree -- otherwise
    # the top-cap M4 pair placed below would get picked up as the new
    # topmost row and throw off the pitch anchor
    next_h = add_side_mounting_holes(tree, delta_height, 11)

    # mounting-hole "top cap": M4 pair nearest J1's row + M3 pad pair just above it
    top_cap_refs = ["H5", "H3", "H7", "H8"]
    for ref in top_cap_refs:
        src = get_fp(tree, ref)

        if src is not None:
            tree.append(clone_footprint(src, 0.0, -delta_height, f"H{next_h}"))
            next_h += 1

    shift_top_cap(tree, delta_height, board_title)
    add_edge_stitching_vias(tree, delta_height)
    add_keying_slots(tree, all_slot_ys)
    center_and_fit_pcb(tree)
    dump_file(tree, out_path)

# ---------- driver ----------

def generate_target(target):
    if target not in SLOT_MANIFESTS:
        raise SystemExit(f"unknown target {target!r}; choices: {list(SLOT_MANIFESTS)}")

    out_dir = HW_ROOT / target
    out_dir.mkdir(exist_ok=True)

    new_slots = SLOT_MANIFESTS[target]
    board_title = target.upper()
    new_symbol_uuids = extend_schematic(new_slots, out_dir / "backplane.kicad_sch")

    extend_pcb(new_slots, out_dir / "backplane.kicad_pcb", new_symbol_uuids, board_title)

    for name in SUPPORT_FILES:
        shutil.copyfile(SRC_DIR / name, out_dir / name)

    print(f"{target}: {len(new_slots)} new slots -> {out_dir}")

def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--target", choices=sorted(SLOT_MANIFESTS), action="append",
                     help="regenerate only this target (repeatable); default is all of them")
    args = ap.parse_args()

    for target in args.target or sorted(SLOT_MANIFESTS):
        generate_target(target)

if __name__ == "__main__":
    main()
