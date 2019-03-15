// Hyperbolic Rogue -- regular honeycombs
// works with spherical and hyperbolic ones -- Euclidean cubic tiling implemented in euclid.cpp
// hyperbolic honeycombs rely on binary:: to deal with floating point errors (just like archimedean)
// Copyright (C) 2011-2019 Zeno Rogue, see 'hyper.cpp' for details

namespace hr {
#if MAXMDIM >= 4
transmatrix cpush(int cid, ld alpha);
transmatrix cspin(int a, int b, ld alpha);
extern
vector<hpcshape> shWall3D, shMiniWall3D;
namespace binary {   
  void build_tmatrix(); 
  void virtualRebaseSimple(heptagon*& base, transmatrix& at);
  int celldistance3(heptagon *c1, heptagon *c2);
  hyperpoint deparabolic3(hyperpoint h);
  }

namespace reg3 {

  map<int, int> close_distances;

  int bucketer(ld x) {
    return int(x * 10 + 100000.5) - 100000;
    }
  
  int bucketer(hyperpoint h) {
    return bucketer(h[0]) + 1000 * bucketer(h[1]) + 1000000 * bucketer(h[2]);
    }
  
  int loop, face;

  vector<hyperpoint> cellshape;
  
  transmatrix spins[12], adjmoves[12];

  ld adjcheck, strafedist;
  bool dirs_adjacent[16][16];

  template<class T> ld binsearch(ld dmin, ld dmax, const T& f) {
    for(int i=0; i<200; i++) {
      ld d = (dmin + dmax) / 2;
      if(f(d)) dmax = d;
      else dmin = d;
      }
    return dmin;
    } 

  void generate() {
  
    using namespace hyperpoint_vec;

    if(S7 == 4) face = 3;
    if(S7 == 6) face = 4;
    if(S7 == 12) face = 5;
    if(S7 == 8) face = 3;
    /* icosahedron not implemented */
    loop = ginf[geometry].tiling_name[5] - '0';
    println(hlog, "face = ", face, " loop = ", loop, " S7 = ", S7);
    
    ld dual_angle = binsearch(0, M_PI, [&] (ld d) {
      hyperpoint h0 = cpush(0, 1) * C0;
      hyperpoint h1 = cspin(0, 1, d) * h0;
      hyperpoint h2 = cspin(1, 2, 2*M_PI/loop) * h1;
      return hdist(h0, h1) > hdist(h1, h2);
      });

    ld dodecahedron_angle = binsearch(0, M_PI, [&] (ld d) {
      hyperpoint h0 = cpush(0, 1) * C0;
      hyperpoint h1 = cspin(0, 1, d) * h0;
      hyperpoint h2 = cspin(1, 2, 2*M_PI/face) * h1;
      return hdist(h0, h1) > hdist(h1, h2);
      });
    
    if(S7 == 8) {
      /* 24-cell is a special case because it is the only one with '4' in the middle of the Schlaefli symbol. */
      /* The computations above assume 3 */
      hyperpoint h1 = hpxy3(.5,.5,.5);
      hyperpoint h2 = hpxy3(.5,.5,-.5);
      dual_angle = hdist(h1, h2);
      }
    
    println(hlog, "dodecahedron angle = ", dodecahedron_angle);
    println(hlog, "dual angle = ", dual_angle);
    
    ld inp_length = binsearch(0, 1.55, [&] (ld d) {
      hyperpoint h = xpush(-d) * spin(2*M_PI/face) * xpush0(d);
      ld alpha = M_PI - atan2(-h[1], h[0]);
      return (alpha < dual_angle / 2) ? hyperbolic : sphere;
      });
    
    println(hlog, "inp length = ", inp_length);
    
    ld edge_length = hdist(xpush0(inp_length), spin(2*M_PI/face) * xpush0(inp_length));
    if(S7 == 8) edge_length = hdist(normalize(hpxyz3(1,1,0,0)), normalize(hpxyz3(1,0,1,0))); 
    println(hlog, "edge length = ", edge_length);
    
    hyperpoint h0 = cpush(0, 1) * C0;
    hyperpoint h1 = cspin(0, 1, dodecahedron_angle) * h0;
    hyperpoint h2 = cspin(1, 2, 2*M_PI/face) * h1;
    hyperpoint h3 = cspin(1, 2, -2*M_PI/face) * h1;
    hyperpoint a2 = S7 == 8 ? normalize(h1 + h2) : normalize(h0 + h1 + h2);
    hyperpoint a3 = S7 == 8 ? normalize(h1 + h3) : normalize(h0 + h1 + h3);
    
    println(hlog, "S7 = ", S7);

    ld whereonline = binsearch(0, 5, [&] (ld d) {
      // sometimes breaks in elliptic
      dynamicval<eGeometry> g(geometry, elliptic ? gCell120 : geometry);
      hyperpoint z2 = a2 * d + C0 * (1-d);
      if(hyperbolic && intval(z2, Hypc) >= 0) return true;
      hyperpoint b2 = normalize(z2);
      hyperpoint z3 = a3 * d + C0 * (1-d);
      hyperpoint b3 = normalize(z3);
      return hdist(b2, b3) >= edge_length;
      });
    
    println(hlog, "whereonline = ", whereonline);
    a2 = normalize(a2 * whereonline + C0 * (1-whereonline));
    a3 = normalize(a3 * whereonline + C0 * (1-whereonline));

    hyperpoint mid = Hypc;
    for(int i=0; i<face; i++) mid += cspin(1, 2, 2*i*M_PI/face) * a2;
    mid = normalize(mid);
    ld between_centers = 2 * hdist0(mid);
    println(hlog, "between_centers = ", between_centers);

    if(S7 == 12 || S7 == 8) {
      spins[0] = Id;
      spins[1] = cspin(0, 1, dodecahedron_angle) * cspin(1, 2, M_PI);
      for(int a=2; a<face+1; a++) spins[a] = cspin(1, 2, 2*M_PI*(a-1)/face) * spins[1];      
      for(int a=S7/2; a<S7; a++) spins[a] = cspin(0, 1, M_PI) * spins[a-S7/2];
      }
    
    if(S7 == 6) {
      spins[0] = Id;
      spins[1] = cspin(0, 1, dodecahedron_angle) * cspin(1, 2, M_PI);
      spins[2] = cspin(1, 2, M_PI/2) * spins[1];
      for(int a=S7/2; a<S7; a++) spins[a] = spins[a-S7/2] * cspin(0, 1, M_PI);
      }
    
    if(S7 == 4) {
      spins[0] = Id;
      spins[1] = cspin(0, 1, dodecahedron_angle) * cspin(1, 2, M_PI);
      for(int a=2; a<face+1; a++) spins[a] = cspin(1, 2, 2*M_PI*(a-1)/face) * spins[1];      
      }
    
    cellshape.clear();
    for(int a=0; a<S7; a++)
    for(int b=0; b<face; b++)
      cellshape.push_back(spins[a] * cspin(1, 2, 2*M_PI*b/face) * a2);
    
    adjmoves[0] = cpush(0, between_centers) * cspin(0, 2, M_PI);
    for(int i=1; i<S7; i++) adjmoves[i] = spins[i] * adjmoves[0];

    for(int a=0; a<S7; a++)
      println(hlog, "center of ", a, " is ", tC0(adjmoves[a]));    
    
    println(hlog, "doublemove = ", tC0(adjmoves[0] * adjmoves[0]));

    adjcheck = hdist(tC0(adjmoves[0]), tC0(adjmoves[1])) * 1.0001;

    int numedges = 0;
    for(int a=0; a<S7; a++) for(int b=0; b<S7; b++) {
      dirs_adjacent[a][b] = a != b && hdist(tC0(adjmoves[a]), tC0(adjmoves[b])) < adjcheck;
      if(dirs_adjacent[a][b]) numedges++;
      }
    println(hlog, "numedges = ", numedges);
    
    if(loop == 4) strafedist = adjcheck;
    else strafedist = hdist(adjmoves[0] * C0, adjmoves[1] * C0);
    }

  void binary_rebase(heptagon *h, const transmatrix& V) {
    }
  
  void test();
    
  struct hrmap_reg3 : hrmap {
  
    heptagon *origin;
    hrmap *binary_map;
    
    unordered_map<heptagon*, pair<heptagon*, transmatrix>> reg_gmatrix;
    unordered_map<heptagon*, vector<pair<heptagon*, transmatrix> > > altmap;

    hrmap_reg3() {
      generate();
      origin = tailored_alloc<heptagon> (S7);
      heptagon& h = *origin;
      h.s = hsOrigin;
      h.cdata = NULL;
      h.alt = NULL;
      h.distance = 0;
      h.c7 = newCell(S7, origin);
      worst_error1 = 0, worst_error2 = 0;
      
      dynamicval<hrmap*> cr(currentmap, this);
      
      heptagon *alt = NULL;
      transmatrix T = Id;
      
      if(hyperbolic) {
        dynamicval<eGeometry> g(geometry, gBinary3); 
        binary::build_tmatrix();
        alt = tailored_alloc<heptagon> (S7);
        alt->s = hsOrigin;
        alt->emeraldval = 0;
        alt->zebraval = 0;
        alt->distance = 0;
        alt->alt = alt;
        alt->cdata = NULL;
        alt->c7 = NULL;
        binary_map = binary::new_alt_map(alt);
        T = xpush(.01241) * spin(1.4117) * xpush(0.1241) * cspin(0, 2, 1.1249) * xpush(0.07) * Id;
        }
      else binary_map = NULL;
      
      reg_gmatrix[origin] = make_pair(alt, T);
      altmap[alt].emplace_back(origin, T);
      
      celllister cl(origin->c7, 4, 100000, NULL);
      for(cell *c: cl.lst) {
        hyperpoint h = tC0(relative_matrix(c->master, origin));
        close_distances[bucketer(h)] = cl.getdist(c);
        }
      }
    
    ld worst_error1, worst_error2;

    heptagon *getOrigin() {
      return origin;
      }
    
    void fix_distances(heptagon *h, heptagon *h2) {
      vector<heptagon*> to_fix;

      auto fix_pair = [&] (heptagon *h, heptagon *h2) {
        if(!h2) return;
        if(h->distance > h2->distance+1) {
          h->distance = h2->distance + 1;
          to_fix.push_back(h);
          }
        else if(h2->distance > h->distance+1) {
          h2->distance = h->distance + 1;
          to_fix.push_back(h2);
          }
        if(h->alt && h->alt == h2->alt) {
          if(altdist(h) > altdist(h2) + 1) {
            altdist(h) = altdist(h2) + 1;
            to_fix.push_back(h);
            }
          else if (altdist(h2) > altdist(h) + 1) {
            altdist(h2) = altdist(h) + 1;
            to_fix.push_back(h2);
            }
          }
        };

      if(!h2) to_fix = {h};
      else fix_pair(h, h2);
      
      for(int i=0; i<isize(to_fix); i++) {
        h = to_fix[i];
        for(int j=0; j<S7; j++) fix_pair(h, h->move(j));
        }
      }
    
    #define DEB 0

    heptagon *create_step(heptagon *parent, int d) {
      auto& p1 = reg_gmatrix[parent];
      if(DEB) println(hlog, "creating step ", parent, ":", d, ", at ", p1.first, tC0(p1.second));
      heptagon *alt = p1.first;
      transmatrix T = p1.second * adjmoves[d];
      transmatrix T1 = T;
      if(hyperbolic) {
        dynamicval<eGeometry> g(geometry, gBinary3);
        dynamicval<hrmap*> cm(currentmap, binary_map);
        binary::virtualRebaseSimple(alt, T);
        }
      
      fixmatrix(T);
      auto hT = tC0(T);
      
      if(DEB) println(hlog, "searching at ", alt, ":", hT);

      if(DEB) for(auto& p2: altmap[alt]) println(hlog, "for ", tC0(p2.second), " intval is ", intval(tC0(p2.second), hT));
      
      ld err;
      
      for(auto& p2: altmap[alt]) if((err = intval(tC0(p2.second), hT)) < 1e-3) {
        if(err > worst_error1) println(hlog, format("worst_error1 = %lg", double(worst_error1 = err)));
        // println(hlog, "YES found in ", isize(altmap[alt]));
        if(DEB) println(hlog, "-> found ", p2.first);
        int fb = 0;
        hyperpoint old = T * (inverse(T1) * tC0(p1.second));
        for(int d2=0; d2<S7; d2++) {
          hyperpoint back = p2.second * tC0(adjmoves[d2]);
          if((err = intval(back, old)) < 1e-3) {
            if(err > worst_error2) println(hlog, format("worst_error2 = %lg", double(worst_error2 = err)));
            if(p2.first->move(d2)) println(hlog, "error: repeated edge");
            p2.first->c.connect(d2, parent, d, false);
            fix_distances(p2.first, parent);
            fb++;
            }
          }
        if(fb != 1) { 
          println(hlog, "found fb = ", fb); 
          println(hlog, old);
          for(int d2=0; d2<S7; d2++) {
            println(hlog, p2.second * tC0(adjmoves[d2]), " in distance ", intval(p2.second * tC0(adjmoves[d2]), old));
            }
          parent->c.connect(d, parent, d, false);
          return parent;
          }
        return p2.first;
        }
      
      if(DEB) println(hlog, "-> not found");
      heptagon *created = tailored_alloc<heptagon> (S7);
      created->c7 = newCell(S7, created);
      created->alt = NULL;
      created->cdata = NULL;
      created->zebraval = hrand(10);
      created->distance = parent->distance + 1;
      fixmatrix(T);
      reg_gmatrix[created] = make_pair(alt, T);
      altmap[alt].emplace_back(created, T);
      created->c.connect(0, parent, d, false);
      return created;
      }

    ~hrmap_reg3() {
      if(binary_map) {        
        dynamicval<eGeometry> g(geometry, gBinary3);
        delete binary_map;
        }
      clearfrom(origin);
      }
    
    map<heptagon*, int> reducers;

    void link_alt(const cellwalker& hs) override {
      auto h = hs.at->master;
      altdist(h) = 0;
      if(h->alt->s != hsOrigin) reducers[h] = hs.spin;
      }
    
    void generateAlts(heptagon* h, int levs, bool link_cdata) override {
      if(reducers.count(h)) {
        heptspin hs(h, reducers[h]);
        reducers.erase(h);
        hs += wstep;
        hs += rev;
        altdist(hs.at) = altdist(h) - 1;
        hs.at->alt = h->alt;
        reducers[hs.at] = hs.spin;
        }
      for(int i=0; i<S7; i++) {
        auto h2 = h->cmove(i);
        if(h2->alt == NULL) {
          h2->alt = h->alt;
          altdist(h2) = altdist(h) + 1;
          fix_distances(h2, NULL);
          }
        }
      }

    void draw() {
      sphereflip = Id;
      
      // for(int i=0; i<S6; i++) queuepoly(ggmatrix(cwt.at), shWall3D[i], 0xFF0000FF);
      
      dq::visited.clear();
      dq::enqueue(viewctr.at, cview());
      
      while(!dq::drawqueue.empty()) {
        auto& p = dq::drawqueue.front();
        heptagon *h = get<0>(p);
        transmatrix V = get<1>(p);
        dynamicval<ld> b(band_shift, get<2>(p));
        bandfixer bf(V);
        dq::drawqueue.pop();
        
        
        cell *c = h->c7;
        if(!do_draw(c, V)) continue;
        drawcell(c, V, 0, false);
    
        for(int i=0; i<S7; i++)
          if(h->move(i))
            dq::enqueue(h->move(i), V * relative_matrix(h->move(i), h));
        }
      }
    
    transmatrix relative_matrix(heptagon *h2, heptagon *h1) {
      auto p1 = reg_gmatrix[h1];
      auto p2 = reg_gmatrix[h2];
      transmatrix T = Id;
      if(hyperbolic) { 
        dynamicval<eGeometry> g(geometry, gBinary3);
        dynamicval<hrmap*> cm(currentmap, binary_map);
        T = binary_map->relative_matrix(p2.first, p1.first);
        }
      return inverse(p1.second) * T * p2.second;
      }
    
    };
  
hrmap* new_map() {
  return new hrmap_reg3;
  }

hrmap_reg3* regmap() {
  return ((hrmap_reg3*) currentmap);
  }

int celldistance(cell *c1, cell *c2) {
  if(c1 == c2) return 0;
  if(c1 == currentmap->gamestart()) return c2->master->distance;
  if(c2 == currentmap->gamestart()) return c1->master->distance;

  auto r = regmap();

  hyperpoint h = tC0(r->relative_matrix(c1->master, c2->master));
  int b = bucketer(h);
  if(close_distances.count(b)) return close_distances[b];

  dynamicval<eGeometry> g(geometry, gBinary3);  
  return 20 + binary::celldistance3(r->reg_gmatrix[c1->master].first, r->reg_gmatrix[c2->master].first);
  }

bool pseudohept(cell *c) {
  auto m = regmap();
  if(sphere) {
    hyperpoint h = tC0(m->relative_matrix(c->master, regmap()->origin));
    if(S7 == 12) {
      hyperpoint h1 = cspin(0, 1, atan2(16, 69) + M_PI/4) * h;
      for(int i=0; i<4; i++) if(abs(abs(h1[i]) - .5) > .01) return false;
      return true;
      }
    if(S7 == 8)
      return h[3] >= .99 || h[3] <= -.99 || abs(h[3]) < .01; 
    if(loop == 3 && face == 3 && S7 == 4)
      return c == m->gamestart();
    if(loop == 4 && face == 3)
      return abs(h[3]) > .9;
    if(loop == 3 && face == 4)
      return abs(h[3]) > .9;
    if(loop == 5 && face == 3)
      return abs(h[3]) > .99 || abs(h[0]) > .99 || abs(h[1]) > .99 || abs(h[2]) > .99;
    }
  // chessboard pattern in 534
  if(geometry == gSpace534) 
    return c->master->distance & 1;
  if(hyperbolic) {
    heptagon *h = m->reg_gmatrix[c->master].first;
    return (h->zebraval == 1) && (h->distance & 1);
    }    
  return false;
  }
#endif

#if 0
/* More precise, but very slow distance. Not used/optimized for now */

ld adistance(cell *c) {  
  hyperpoint h = tC0(regmap()->reg_gmatrix[c->master].second);
  h = binary::deparabolic3(h);
  return regmap()->reg_gmatrix[c->master].first->distance * log(2) - h[0];
  }

unordered_map<pair<cell*, cell*>, int> memo;

bool cdd;

int celldistance(cell *c1, cell *c2) {
  if(memo.count(make_pair(c1, c2))) return memo[make_pair(c1, c2)];
  if(c1 == c2) return 0;
  vector<cell*> v[2];
  v[0].push_back(c1);
  v[1].push_back(c2);
  
  int steps = 0;
  
  map<cell*, int> visited;
  visited[c1] = 1;
  visited[c2] = 2;
  
  while(true) {
    if(cdd) {
      println(hlog, "state ", steps, "/",isize(v[0]), "/", isize(v[1]));
      println(hlog, "  A: ", v[0]);
      println(hlog, "  B: ", v[1]); 
      }
    for(int i: {0,1}) {
      vector<cell*> new_v;
      for(cell *c: v[i]) forCellCM(cn, c) if(adistance(cn) < adistance(c)) {
        auto &vi = visited[cn];
        if((vi&3) == 0) {
          vi = 4 * (steps+1);
          vi |= (1<<i);
          new_v.push_back(cn);
          }
        else if((vi&3) == 2-i) {
          vector<pair<cell*, int>> ca1, ca2;
          int b1 = 4*steps-4;
          int b2 = ((vi>>2)<<2) - 4;
          for(auto p: visited) {
            if(cdd) println(hlog, p);
            int ps = p.second & 3;
            if(ps == 1+i && p.second >= b1)
              ca1.emplace_back(p.first, p.second/4);
            if(ps == 2-i && p.second >= b2 && p.second <= b2+8)
              ca2.emplace_back(p.first, p.second/4);
            }
          int bound = 1<<16;
          for(auto p1: ca1) for(auto p2: ca2) {
            hyperpoint h = tC0(relative_matrix(p1.first->master, p2.first->master));
            int b = bucketer(h);
            if(close_distances.count(b)) {
              int d = close_distances[b] + p1.second + p2.second;
              if(cdd) println(hlog, "candidate: close=", close_distances[b], p1, p2, "; h = ", h);
              if(d < bound) bound = d;
              }
            else if(cdd) println(hlog, "bucket missing");
            }
          return memo[make_pair(c1, c2)] = bound;
          return bound;
          }
        }
      v[i] = std::move(new_v);
      }
    steps++;
    }
  }

cellwalker target;
int tsteps;

int dist_alt(cell *c) {
  if(!target.at) {
    target = cellwalker(currentmap->gamestart(), 0);
    tsteps = 0;
    for(int i=0; i<30; i++) target += wstep, target += rev, tsteps++;
    }
  if(specialland == laCamelot) return reg3::celldistance(c, target.at);
  else {
    int d = reg3::celldistance(c, target.at) - tsteps;
    if(d < 10) target += wstep, target += rev, tsteps++;
    return d;
    }
  }
#endif

// Construct a cellwalker in direction j from cw.at, such that its direction is as close
// as possible to cw.spin. Assume that j and cw.spin are adjacent

cellwalker strafe(cellwalker cw, int j) {
  hyperpoint hfront = tC0(adjmoves[cw.spin]);
  transmatrix T = currentmap->relative_matrix(cw.at->cmove(j)->master, cw.at->master);
  for(int i=0; i<S7; i++) if(i != cw.at->c.spin(j))
    if(hdist(hfront, T * tC0(adjmoves[i])) < strafedist + .01)
      return cellwalker(cw.at->move(j), i);
  println(hlog, "incorrect strafe");
  exit(1);
  }

}
}

