import pdb
from pprint import pprint

FINAL_STATE = 0x1ef

def xor_mask_from_pip(pip):
  xor_mask = 0xffffffff
  if pip == 1: xor_mask = 0x01b
  if pip == 2: xor_mask = 0x007
  if pip == 3: xor_mask = 0x036
  if pip == 4: xor_mask = 0x049
  if pip == 5: xor_mask = 0x0ba
  if pip == 6: xor_mask = 0x124
  if pip == 7: xor_mask = 0x0d8
  if pip == 8: xor_mask = 0x1c0
  if pip == 9: xor_mask = 0x1b0

  return xor_mask


class PipSolutionStep:
  state = -1
  resolving_pip = -1
  resolving_SolStep = None

  def __repr__(self):
    if self.resolving_SolStep:
      s = ','.join(["state=0x%03x" % self.state,
                    "resolving_pip=%d" % self.resolving_pip,
                    "resolving_SolStep.state=0x%03x" % self.resolving_SolStep.state])
    else:
      s = ','.join(["state=0x%03x" % self.state,
                    "resolving_pip=-1",
                    "resolving_SolStep.state=None"])
    return s


# Empty solution table
solved_states = 0x200 * [[]]
for i,m in enumerate(solved_states):
  solved_states[i] = PipSolutionStep()

solved_states[FINAL_STATE].state = FINAL_STATE
FINAL_SOLUTION_STEP = solved_states[FINAL_STATE]


def depth(state):
  d = 0
  done = False
  current_state = solved_states[state]

  while not done:
    if current_state == FINAL_SOLUTION_STEP:
      done = True
    else:
      current_state = current_state.resolving_SolStep
      d = d + 1

  return d



def suboptimal_init_solution_table():
  def fill_solution_table_entry(state):
    work_list = []

    for pip in range(1, 10):
      next_state = state ^ xor_mask_from_pip(pip)

      if solved_states[next_state] == FINAL_SOLUTION_STEP:
        continue

      # Populate if entry is empty
      if not solved_states[next_state].resolving_SolStep:
        solved_states[next_state].state = next_state
        solved_states[next_state].resolving_pip = pip
        solved_states[next_state].resolving_SolStep = solved_states[state]
        work_list.append(next_state)

    for work_list_state in work_list:
      fill_solution_table_entry(work_list_state)

  fill_solution_table_entry(FINAL_STATE)

# Complete initial population
suboptimal_init_solution_table()

#pprint(solved_states)

#print(depth(0x1ef))
#print(depth(0x1f3))

iterations = 0

print("Initial solution")

done = False
while not done:
  sorted_states = sorted([(i, depth(i)) for i,e in enumerate(solved_states)], key=lambda x:x[1])
  iterations = iterations + 1
  updated_count = 0

  for state, state_depth in sorted_states[1:]:
    best_move_depth = depth(state)
    best_move_pip = solved_states[state].resolving_pip
    updated = False

    for pip in range(1, 10):
      next_state = state ^ xor_mask_from_pip(pip)

      if solved_states[next_state] == FINAL_SOLUTION_STEP:
        continue

      if depth(next_state) + 1 < best_move_depth:
        updated = True
        updated_count = updated_count + 1
        best_move_depth = depth(next_state) + 1
        best_move_pip = pip

    if updated:
      print("%d Updated state 0x%03x (old_depth=%d, new_depth=%d)" % (
        iterations, state, state_depth, best_move_depth))
      solved_states[state].resolving_pip = best_move_pip
      solved_states[state].resolving_SolStep = solved_states[state ^ xor_mask_from_pip(best_move_pip)]

  if updated_count > 0:
    print("%d changes" % updated_count)

  if not updated:
    done = True

#pprint(solved_states)

for i, ss in enumerate(solved_states):
  print("{.best_pip_choice = %d, .tree_depth = %d}, // 0x%03x(%d) --> 0x%03x(%d)" %
          (ss.resolving_pip, depth(ss.state), i, i,
              i ^ xor_mask_from_pip(ss.resolving_pip),
              i ^ xor_mask_from_pip(ss.resolving_pip)))

  #  print("let depthToSolution[%d] = %d; // resolving_pip = %d, 0x%03x(%d) --> 0x%03x(%d)" %
  #      (ss.state, depth(ss.state), ss.resolving_pip, ss.state, ss.state,
  #      i ^ xor_mask_from_pip(ss.resolving_pip),
  #      i ^ xor_mask_from_pip(ss.resolving_pip)))
