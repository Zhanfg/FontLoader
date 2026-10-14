import unittest
from types import SimpleNamespace
from pathlib import Path
from tempfile import TemporaryDirectory
import sys
sys.path.insert(0,"tools")
from vf_audit import validate_axes, inspect_fallback

class VariableFontAuditorTest(unittest.TestCase):
    def axis(self,tag,lo=100,default=400,hi=900):
        return SimpleNamespace(axisTag=tag,minValue=lo,
                               defaultValue=default,maxValue=hi)

    def test_wght_ital(self):
        axes,errors=validate_axes([self.axis("wght"),self.axis("ital",0,0,1)])
        self.assertEqual(errors,[])
        self.assertEqual(axes[0]["default"],400)
    def test_reject_wrong_ranges(self):
        axes,errors=validate_axes([self.axis("wght",900,400,100)])
        self.assertTrue(errors)
    def test_duplicate_tag(self):
        axes,errors=validate_axes([self.axis("wght"),self.axis("wght")])
        self.assertTrue(errors)
    def test_xml_axes(self):
        with TemporaryDirectory() as folder:
            p=Path(folder)/"font_fallback.xml"
            p.write_text('<familyset><family name="sans-serif" supportedAxes="wght,ital"/></familyset>')
            self.assertEqual(inspect_fallback(p)["families"][0]["errors"],[])
            p.write_text('<familyset><family supportedAxes="wght,opsz"/></familyset>')
            self.assertTrue(inspect_fallback(p)["families"][0]["errors"])

if __name__=="__main__":
    unittest.main()
