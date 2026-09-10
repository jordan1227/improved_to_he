"""Run from any directory. Static dialogue contracts, not engine acceptance."""
from pathlib import Path
import re
import subprocess
import xml.etree.ElementTree as ET

ROOT = Path(__file__).resolve().parents[2]
CONFIG = ROOT / 'gamedata/config'
SOURCE = ROOT / 'gamedata/scripts/he_pda_access.script'

def text(path):
    return path.read_bytes().decode('cp1251')

dialogues = ET.fromstring('<root>' + text(CONFIG / 'gameplay/dialogs_he_pda_access.xml') + '</root>')
languages = {}
for language in ('rus', 'eng'):
    root = ET.fromstring((CONFIG / f'text/{language}/script_strings.xml').read_bytes())
    keys = [node.get('id') for node in root.findall('string') if node.get('id', '').startswith('he_pda_')]
    assert len(keys) == len(set(keys)), ('duplicate localization', language)
    languages[language] = set(keys)

source = text(SOURCE)
# Cover PDA lines outside this fragment too (Wolf/Petruha and charging services).
references = set()
for path in (CONFIG / 'gameplay').glob('*.xml'):
    references.update(re.findall(r'<text>\s*(he_pda_[^<\s]+)\s*</text>', text(path)))
for path in (ROOT / 'gamedata/scripts').rglob('*.script'):
    references.update(re.findall(r'(?:translate_string|local_notice)\("(he_pda_[^"]+)"', text(path)))
for language in languages:
    keys = set()
    for path in (CONFIG / f'text/{language}').glob('*.xml'):
        keys.update(re.findall(r'<string\s+id="(he_pda_[^"]+)"', text(path)))
    assert not references-keys, ('missing PDA reference', language, references-keys)
functions = set(re.findall(r'function\s+(\w+)\(', source))
profiles = '\n'.join(text(CONFIG / ('gameplay/' + name)) for name in
                     ('character_desc_escape.xml', 'character_desc_bar.xml', 'character_desc_nlc30_spawn.xml'))
info_definitions = '\n'.join(text(path) for path in (CONFIG / 'gameplay').glob('info*.xml'))
ids = [d.get('id') for d in dialogues]
assert len(ids) == len(set(ids)), 'duplicate dialogue ID'
for dialog in dialogues:
    ident = dialog.get('id')
    assert f'<actor_dialog>{ident}</actor_dialog>' in profiles, ('missing character binding', ident)
    phrases = dialog.findall('phrase_list/phrase')
    nodes = {p.get('id'): p for p in phrases}
    assert len(nodes) == len(phrases), ('duplicate phrase', ident)
    depth, todo = {}, [('0', 0)]
    while todo:
        key, parity = todo.pop()
        assert key in nodes, ('missing target', ident, key)
        if key in depth:
            assert depth[key] == parity, ('speaker parity conflict', ident, key)
            continue
        depth[key] = parity
        todo.extend((n.text, 1-parity) for n in nodes[key].findall('next'))
    assert set(depth) == set(nodes), ('unreachable phrases', ident, set(nodes)-set(depth))
    for node in dialog.iter():
        value = (node.text or '').strip()
        if node.tag == 'text' and value:
            for language, keys in languages.items():
                assert value in keys, ('missing text', ident, language, value)
        elif node.tag in ('precondition', 'action', 'init_func') and value.startswith('he_pda_access.'):
            assert value.split('.')[1] in functions, ('missing callback', value)
        elif node.tag in ('has_info', 'dont_has_info', 'give_info', 'disable_info'):
            assert f'id="{value}"' in info_definitions, ('undeclared info', value)
    for phrase in phrases:
        if any((a.text or '').startswith(('he_pda_access.buy_', 'he_pda_access.upgrade_pda',
                                         'he_pda_access.confirm_reflash', 'he_pda_access.install_notes_module'))
               for a in phrase.findall('action')):
            successors = [nodes[n.text] for n in phrase.findall('next')]
            conditions = {p.findtext('precondition') for p in successors}
            assert all('he_pda_access.last_transaction_' + state in conditions
                       for state in ('pending', 'success', 'failed')), ('result coverage', ident, phrase.get('id'))

trading = text(CONFIG / 'gameplay/dialogs_trading.xml')
assert 'gameplay\\dialogs_he_pda_access.xml' in trading
system = text(CONFIG / 'system.ltx')
for registry in ('dialogs_trading', 'script_strings', 'npc_profile', 'npc_profile_nlc30_spawn'):
    assert registry in system, ('unregistered root', registry)

# Prices have distinct callbacks; a localization edit cannot silently change payment.
by_id = {d.get('id'): d for d in dialogues}
assert by_id['he_pda_barman_purchase'].find("phrase_list/phrase[@id='2']/action").text == 'he_pda_access.buy_barman_base'
assert by_id['he_pda_sidor_inquiry'].find("phrase_list/phrase[@id='2']/action").text == 'he_pda_access.buy_base'
assert by_id['he_pda_sidor_inquiry'].find("phrase_list/phrase[@id='12']/action").text == 'he_pda_access.buy_sidor_base'
for phrase_id in ('10', '40'):
    assert by_id['he_pda_sidor_inquiry'].find(f"phrase_list/phrase[@id='{phrase_id}']/action").text == 'he_pda_access.mark_sidor_discount'

compiler = ROOT.parent / 'Tools/AXRToolset-master/bin/luac5.1.exe'
assert compiler.is_file(), f'Lua 5.1 compiler unavailable: {compiler}'
for path in (SOURCE, ROOT / 'gamedata/scripts/sivol/sivol_emp_config.script',
             ROOT / 'gamedata/scripts/sivol/sivol_pda.script',
             ROOT / 'gamedata/scripts/sivol/sivol_critical_malfunction.script',
             ROOT / 'gamedata/scripts/sak/sak.script', ROOT / 'docs/tests/pda_state_tests.lua'):
    subprocess.run([str(compiler), '-p', str(path)], check=True)
print(f'PASS: {len(ids)} dialogue graphs, speaker parity, bindings, RU/EN keys, callbacks, info IDs, prices and Lua 5.1 parsing.')
