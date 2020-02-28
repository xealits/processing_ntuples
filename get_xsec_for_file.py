import argparse
import logging

import xml.etree.ElementTree as ET

'''
https://stackoverflow.com/a/1912483/1420489
Example how the ElementTree works:
>>> len(hps)
259
>>> hps[0]
<Element 'hard_process' at 0x7f41e23bd110>
>>> hps[0].get('xsec')
'18610.0'
>>> hps[0].children
Traceback (most recent call last):
File "<input>", line 1, in <module>
hps[0].children
AttributeError: 'Element' object has no attribute 'children'
>>> hps[0].findall('dset')
[<Element 'dset' at 0x7f41e23bd090>]
>>> hps[0].findall('dset')[0]
<Element 'dset' at 0x7f41e23bd090>
>>> hps[0].findall('dset')[0].get('dtag')
'MC2016_Summer16_DYJetsToLL_10to50_amcatnlo'
'''


class DsetsInfo:
    '''DsetsInfo handles the access to the dsets_info.xml file from Python'''

    def __init__(self, dsets_info_filename='dsets_info_3.xml'):
        '''__init__(self, dsets_info_filename='dsets_info_3.xml')

        dsets_info_filename -- the path to the XML file
        '''

        self._dsets_info_file = dsets_info_filename
        self._root = ET.parse(self._dsets_info_file).getroot()
        self._hps  = self._root.findall('hard_process')

    def match_hp_by_dtag(self, filename):
        """match_hp_by_dtag(self, filename)

        filename is a string that contains a dtag, like
        MC2017legacy_Fall17_WJets_madgraph_v2_1.root
        """

        for hp in self._hps:
            for dset in hp.findall('dset'):
                if dset.get('dtag') in filename:
                    return hp, dset.get('dtag') # hp.get('xsec') is the xsec
        return None


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        formatter_class = argparse.RawDescriptionHelpFormatter,
        description = "example of how to parse the XML dsets_info file",
        epilog = "Example:\n$ python get_xsec_for_file.py gstore_outdirs/94v15/WJetsToLNu_TuneCP5_13TeV-madgraphMLM-pythia8/Ntupler_94v15_MC2017legacy_Fall17_WJets_madgraph_v2/200225_120706/0000/MC2017legacy_Fall17_WJets_madgraph_v2_1.root"
        )

    parser.add_argument("data_file",       help="a file with some data that needs x-sections")
    parser.add_argument("-d", "--debug",   action='store_true', help="DEBUG level of logging")


    args = parser.parse_args()

    if args.debug:
        logging.basicConfig(level=logging.DEBUG)
    else:
        logging.basicConfig(level=logging.INFO)

    logging.debug("parsed args: %s" % repr(args))


    dsets_info = DsetsInfo()
    matched_hp = dsets_info.match_hp_by_dtag(args.data_file)

    if matched_hp is None:
        logging.error('did not match any dtag to the filename %s' % args.data_file)
    else:
        print(matched_hp[1], float(matched_hp[0].get('xsec')))

