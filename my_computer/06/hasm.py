# Hack computer assembler - NAND2Tetrix P1 - 2017
import sys
import pdb

auto_variable_val = 16

symbols = {
  'R0' : 0,
  'R1' : 1,
  'R2' : 2,
  'R3' : 3,
  'R4' : 4,
  'R5' : 5,
  'R6' : 6,
  'R7' : 7,
  'R8' : 8,
  'R9' : 9,
  'R10' : 10,
  'R11' : 11,
  'R12' : 12,
  'R13' : 13,
  'R14' : 14,
  'R15' : 15,
  'SP' : 0,
  'LCL' : 1,
  'ARG' : 2,
  'THIS' : 3,
  'THAT' : 4,
  'SCREEN' : 16384,
  'KBD' : 24576,
  }

class ParseError(Exception):
  pass

def baseN(num,b,numerals="0123456789abcdefghijklmnopqrstuvwxyz",length=0):
  s = ((num == 0) and numerals[0]) or (baseN(num // b, b, numerals).lstrip(numerals[0]) + numerals[num % b])
  return '0' * (length - len(s)) + s

def validSymbol(sym):
  s = set(sym)
  v = set('abcdefghijklmnopqrstuvwyxzABCDEFGHIJKLMNOPQRSTUVWYXZ1234567890_.$:')
  if s - v:
    return False
  if sym[0] in '1234567890':
    return False
  return True

def test_case():
  c = [ ["  // my program", ""],
        ["// this", ""],
        ["    @24576  // read keyboard",                   "0110000000000000"],
        ["    D=M   ",                                     "1111110000010000"],
        ["    @9",                                         "0000000000001001"],
        ["    D;JEQ   // no keypress? set to white color", "1110001100000010"],
        ["    D=-1",                                       "1110111010010000"],
        ["    @0",                                         "0000000000000000"],
        ["    M=D",                                        "1110001100001000"],
        ["    @13   ",                                     "0000000000001101"],
        ["    0;JMP",                                      "1110101010000111"],
        ["    @0",                                         "0000000000000000"],
        ["    D=A",                                        "1110110000010000"],
        ["    @0",                                         "0000000000000000"],
        ["    M=D",                                        "1110001100001000"],
        ["    @16384",                                     "0100000000000000"],
        ["    D=A",                                        "1110110000010000"],
        ["    D       // noop",                            "1110001100000000"],
        ["    @1",                                         "0000000000000001"],
        ["    M=D     // set screen base address",         "1110001100001000"],
        ["    @0     // load color",                       "0000000000000000"],
        ["    D=M",                                        "1111110000010000"],
        ["    @1",                                         "0000000000000001"],
        ["    A=M     // load screen address",             "1111110000100000"],
        ["    M=D     // update screen pixels",            "1110001100001000"],
        ["    @1",                                         "0000000000000001"],
        ["    D=M",                                        "1111110000010000"],
        ["    D=D+1   // increment screen address",        "1110011111010000"],
        ["    @1",                                         "0000000000000001"],
        ["    M=D",                                        "1110001100001000"],
        ["    @24575  // last screen location",            "0101111111111111"],
        ["    D=A",                                        "1110110000010000"],
        ["    @1",                                         "0000000000000001"],
        ["    D=D-M",                                      "1111010011010000"],
        ["    @17",                                        "0000000000010001"],
        ["    D;JGE   // continue if not done",            "1110001100000011"],
        ["    @0",                                         "0000000000000000"],
        ["    0;JMP   // restart program",                 "1110101010000111"],
        ["    D=A+1;JLE",                                  "1110110111010110"],
        ["    M;JGE",                                      "1111110000000011"],
      ]
  for i in c:
    yield i

def normalize_instruction(s):
  instr = s.strip()

  if instr.find('//') >= 0:
    instr = instr[:instr.find('//')]

  instr = instr.replace(' ', '')

  return instr

def assemble(s):
  global auto_variable_val, symbols
  # A Instruction encoding
  if s[0] == '@':
    a_target_str = s[1:]

    if a_target_str in symbols:
      a_target_str = symbols[a_target_str]

    is_int = False
    try:
      a_target_num = int(a_target_str)
      is_int = True
    except ValueError:
      pass

    if not is_int:
      if not validSymbol(a_target_str):
        raise ParseError("Parse error: invalid character in variable definition '%s'" % symbol)
      symbols[a_target_str] = auto_variable_val
      a_target_num = symbols[a_target_str]
      auto_variable_val = auto_variable_val + 1
      is_int = True

    if is_int:
      if a_target_num < 0:
        raise ParseError("Parse error: invalid integer %s" % a_target_num)
      if a_target_num > 0x7fff:
        raise ParseError("Parse error: invalid integer %s (too big)" % a_target_num)

      s = baseN(a_target_num, 2, length=16)
      return s

  # C Instruction encoding
  else:
    preamble = '111'
    aval = '0'
    cval = '000000'
    dval = '000'
    jval = '000'

    # Is jump instruction?
    if ';' in s:
      comp = s[:s.find(';')]
      jump = s[s.find(';') + 1:]
      if   jump == 'JGT': jval = '001'
      elif jump == 'JEQ': jval = '010'
      elif jump == 'JGE': jval = '011'
      elif jump == 'JLT': jval = '100'
      elif jump == 'JNE': jval = '101'
      elif jump == 'JLE': jval = '110'
      elif jump == 'JMP': jval = '111'
      else:
        raise ParseError("Parse error: expecting jmp instruction '%s'" % s)

      # Remove the jump portion from the instruction during further evaluation
      s = s[:s.find(';')]
      
    if '=' in s:
      dest = s[:s.find('=')]
      comp = s[s.find('=') + 1:]

      if   dest == 'M':   dval = '001'
      elif dest == 'D':   dval = '010'
      elif dest == 'MD':  dval = '011' # Strict interpretation of Fig. 4.4
      elif dest == 'A':   dval = '100'
      elif dest == 'AM':  dval = '101'
      elif dest == 'AD':  dval = '110'
      elif dest == 'AMD': dval = '111'
      else:
        raise ParseError("Parse error: expecting destination instruction '%s'" % s)
    else:
      comp = s

    if   comp == '0':   cval = '101010'
    elif comp == '1':   cval = '111111'
    elif comp == '-1':  cval = '111010'
    elif comp == 'D':   cval = '001100'
    elif comp == 'A':   cval = '110000'
    elif comp == '!D':  cval = '001101'
    elif comp == '!A':  cval = '110001'
    elif comp == '-D':  cval = '001111'
    elif comp == '-A':  cval = '110011'
    elif comp == 'D+1': cval = '011111'
    elif comp == 'A+1': cval = '110111'
    elif comp == 'D-1': cval = '001110'
    elif comp == 'A-1': cval = '110010'
    elif comp == 'D-1': cval = '001110'
    elif comp == 'A-1': cval = '110010'
    elif comp == 'D+A': cval = '000010'
    elif comp == 'D-A': cval = '010011'
    elif comp == 'A-D': cval = '010011'
    elif comp == 'D&A': cval = '000000'
    elif comp == 'D|A': cval = '010101'

    elif comp == 'M':   cval = '110000'; aval = '1'
    elif comp == '!M':  cval = '110001'; aval = '1'
    elif comp == 'M+1': cval = '110111'; aval = '1'
    elif comp == 'M-1': cval = '110010'; aval = '1'
    elif comp == 'D+M': cval = '000010'; aval = '1'
    elif comp == 'D-M': cval = '010011'; aval = '1'
    elif comp == 'M-D': cval = '000111'; aval = '1'
    elif comp == 'D&M': cval = '000000'; aval = '1'
    elif comp == 'D|M': cval = '010101'; aval = '1'
    else:
      raise ParseError("Parse error: unknown computation instruction '%s'" % s)

  return preamble + aval + cval + dval + jval

def test_all():
  for case in test_case():
    inline = case[0]
    outline = case[1]

    inline = normalize_instruction(inline)

    if not inline:
      continue

    ml_instr = assemble(inline)

    if ml_instr != outline:
      print "ERROR: %s" % inline
      print "expected: %s" % outline
      print "actual: %s" % ml_instr

  print 'done'

class ListingLine:
  asm_line_number = 0
  is_instruction = False
  is_label = False
  has_comment = False
  instruction_number = 0
  instruction_text = ''
  ml_instr = ''
  label_text = ''
  comment_text = ''

  def __repr__(self):
    return "%i %s %s %s %i '%s' '(%s)' '%s'" % (
        self.asm_line_number, self.is_instruction, self.is_label,
        self.has_comment, self.instruction_number, self.instruction_text,
        self.ml_instr, self.label_text, self.comment_text)

def main():
  global auto_variable_val, symbols

  if len(sys.argv) == 1:
    print 'Usage: python hasm.py [-l] FILE.asm'
    sys.exit(-1)

  # Check for listing file production
  if sys.argv[1] == '-l':
    if len(sys.argv) == 2:
      print 'Usage: python hasm.py [-l] FILE.asm'
      sys.exit(-1)
    make_listing_file = True
    filename = sys.argv[2]
  else:
    make_listing_file = False
    filename = sys.argv[1]

  if make_listing_file:
    print "---"
    print "--- Hack Computer"
    print "--- Listing file: %s" % sys.argv[-1]
    print "---\n"
    print "--- CODE LISTING ---\n"

  default_symbols = set(symbols.keys())

  lines = file(filename).read().split('\n')
  listingLines = []
  statement_no = 0
  for i,line in enumerate(lines):
    listingLine = ListingLine()
    listingLine.asm_line_number = i
    line = line.strip()
    if line.find('//') >= 0:
      listingLine.comment_text = line[line.find('//'):]
      line = line[:line.find('//')].strip()
      listingLine.has_comment = True
    if line:
      if line[0] == '(':
        if line[-1] != ')':
          raise ParseError("Parse error: malformed symbol '%s'" % line)
        symbol = line[1:-1]
        if not validSymbol(symbol):
          raise ParseError("Parse error: invalid character in '%s'" % symbol)
        listingLine.is_label = True
        listingLine.label_text = symbol
        symbols[symbol] = statement_no
        listingLines.append(listingLine)
        continue
      listingLine.is_instruction = True
      listingLine.instruction_number = statement_no
      listingLine.instruction_text = line
      statement_no = statement_no + 1
      listingLines.append(listingLine)
    else:
      listingLines.append(listingLine)

  code_symbols = set(symbols.keys())

  for i,listingLine in enumerate(listingLines):
    if listingLine.is_instruction:
      try:
        ml_instr = assemble(listingLine.instruction_text)
        listingLines[i].ml_instr = ml_instr
        if not make_listing_file:
          print ml_instr
      except ParseError as e:
        print 'Line %d' % i
        print e
        sys.exit(1)

  data_symbols = set(symbols.keys()) - code_symbols

  if make_listing_file:
    for i,listingLine in enumerate(listingLines):
      if listingLine.is_instruction:
        asm_hex = int(listingLine.ml_instr, 2)
        if listingLine.is_instruction:
          a_instr_value = ''

          if listingLine.instruction_text[0] == '@':
            a_instr = listingLine.instruction_text[1:]
            if a_instr not in default_symbols:
              try:
                throw_away = int(a_instr)
              except:
                a_instr_value = " =%d" % symbols[a_instr]
        if listingLine.has_comment:
          # Move inline comments to its own line for readability
          print "%5i: %4s      %s" % (listingLine.instruction_number, "",
              listingLine.comment_text)
        print "%5i: %04X   %-35s %s%-8s%s" % (
            listingLine.instruction_number, asm_hex,
            listingLine.instruction_text, "",
            a_instr_value, listingLine.ml_instr)
      elif listingLine.has_comment:
        print "              %s" % listingLine.comment_text
      elif listingLine.is_label:
        print ""
        symbol_val = symbols[listingLine.label_text]
        print "      (%s) =%d" % (listingLine.label_text, symbol_val)

    print ""
    print "--- DATA SYMBOLS ---\n"

    data_symbol_tuples = {k: symbols[k] for k in data_symbols}
    data_symbol_tuples = [(v,k) for k,v in data_symbol_tuples.iteritems()]

    for v,k in sorted(data_symbol_tuples):
      print "%4d  %s" % (v, k)

    print ""
    print "--- CODE SYMBOLS ---\n"

    new_code_symbols = code_symbols - default_symbols
    code_symbol_tuples = {k: symbols[k] for k in new_code_symbols}
    code_symbol_tuples = [(v,k) for k,v in code_symbol_tuples.iteritems()]

    for v,k in sorted(code_symbol_tuples):
      print "%4d  %s" % (v, k)


if __name__ == '__main__':
  main()
  #test_all()
