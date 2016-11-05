#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#ifdef _MSC_VER
#include <conio.h>
#endif
#include <unordered_map>

// Based on implementation by Chris Lomont, www.lomont.org, public domain.
// From game programming gems 7.
class WELL512
{
public:
	unsigned long mState[16];
	unsigned int mIndex;

	WELL512()
	{
		mIndex = 0;
		srand(0);
	}

	void srand(int aSeed)
	{
		int i;
		for (i = 0; i < 16; i++)
		{
			mState[i] = 0x7457edb7 + (i + 1) * aSeed;
		}
	}

	unsigned long rand()
	{
		unsigned long a, b, c, d;
		a = mState[mIndex];
		c = mState[(mIndex + 13) & 15];
		b = a ^ c ^ (a << 16) ^ (c << 15);
		c = mState[(mIndex + 9) & 15];
		c ^= (c >> 11);
		a = mState[mIndex] = b ^ c;
		d = a ^ ((a << 5) & 0xDA442D24UL);
		mIndex = (mIndex + 15) & 15;
		a = mState[mIndex];
		mState[mIndex] = a ^ b ^ d ^ (a << 2) ^ (b << 18) ^ (c << 28);
		return mState[mIndex];
	}
};

int gMaxDepth = 6;

struct prevkeys
{
	int a, b, c, d, e, f;
	prevkeys()
	{
		reset();
	}
	void reset()
	{
		a = b = c = d = e = f = -1;
	}
	void shift()
	{
		if (gMaxDepth > 5) f = e;
		if (gMaxDepth > 4) e = d;
		if (gMaxDepth > 3) d = c;
		if (gMaxDepth > 2) c = b;
		if (gMaxDepth > 1) b = a;
		a = -1;
	}
	int depth()
	{
		if (f != -1) return 6;
		if (e != -1) return 5;
		if (d != -1) return 4;
		if (c != -1) return 3;
		if (b != -1) return 2;
		if (a != -1) return 1;
		return 0;
	}
	void sink()
	{
		if (f != -1)
		{
			f = -1;
		}
		else
		if (e != -1)
		{
			e = -1;
		}
		else
		if (d != -1)
		{
			d = -1;
		}
		else
		if (c != -1)
		{
			c = -1;
		}
		else
		if (b != -1)
		{
			b = -1;
		}
		else
		{
			a = -1;
		}
	}
	bool operator==(const prevkeys &other) const
	{
		return (a == other.a) && (b == other.b) && (c == other.c) && (d == other.d) && (e == other.e) && (f == other.f);
	}
};

namespace std {

	template <>
	struct hash<prevkeys>
	{
		std::size_t operator()(const prevkeys& k) const
		{
			return (k.a << 1)^(k.b << 3)^(k.c << 2)^(k.d)^(k.e << 3)^(k.f << 1);
		}
	};

}

int whitespace(char aCharacter)
{
	switch (aCharacter)
	{
	case ' ':
	case '\t':
	case '\r':
	case '\n':
		return 1;
	}
	return 0;
}

int is_numeric(char *aString)
{
	if (!*aString) return 0;
	while (*aString)
	{
		if (*aString < '0' || *aString > '9')
			return 0;
		aString++;
	}
	return 1;
}

int is_alpha(char *aChar)
{
	if (!*aChar) return 0;
	if (toupper(*aChar) < 'A' || toupper(*aChar) > 'Z')
		return 0;
	return 1;
}


class WordCounter
{
public:
	std::unordered_map<int, char*> mWord;
	std::unordered_map<int, int> mFlags, mHits, mHash, mUsed;
	std::unordered_map<prevkeys, std::unordered_map<int, int>> mLongHit;
	char *mSentence[1024];	
	prevkeys mPrevKeys;
	int mPrevToken;
	int mTokens;
	int mWordno;
	int mSentences;

	int calcHash(char *aString)
	{
		unsigned int i = 0;
		while (*aString)
		{
			i = (i << 11) | (i >> 21);
			i ^= *aString;
	        aString++;
		}    
    
		return i;
	}    

	WordCounter()
	{
		mPrevToken = -1;
		mTokens = 0;
		mWordno = 0;		
		mSentences = 0;
	}

	void tokenRef(int aCurrent)
	{
		prevkeys t = mPrevKeys;
		while (t.depth())
		{
			mLongHit[t][aCurrent]++;
			t.sink();
		}

		mPrevKeys.shift();
		mPrevKeys.a = aCurrent;

		if (mWordno == 0)
			mPrevKeys.reset();
	}

	void addWordcountToken(char *aToken, int aFirst, int aLast)
	{
		int h = calcHash(aToken);
		int i;
		if (aToken == NULL || *aToken == 0)
			return;

		for (i = 0; i < mTokens; i++)
		{
			if (mHash[i] == h && strcmp(mWord[i], aToken) == 0)
			{
				tokenRef(i);
				mHits[i]++;
				mFlags[i] |= (aLast) ? 2 : (aFirst) ? 1 : 0;
				return;
			}
		}

		mWord[mTokens] = _strdup(aToken);
		mHash[mTokens] = h;
		mHits[mTokens] = 1;
		mFlags[mTokens] = (aLast) ? 2 : (aFirst) ? 1 : 0;
		tokenRef(mTokens);
		mTokens++;
	}

	int valid(char *aToken, int &aEnd)
	{
		int allupper = 1;
		int len = 0;
		while (*aToken)
		{
			if ((*aToken >= 'a' && *aToken <= 'z') ||
				(*aToken >= 'A' && *aToken <= 'Z') ||
				*aToken == '\'' ||
				*aToken == '´' ||
				*aToken == ':' ||
				*aToken == ';' ||
				*aToken == '.' ||
				*aToken == '?' ||
				*aToken == '!')
			{
				// Probably valid.
				if ((*aToken == ':' ||
					*aToken == ';' ||
					*aToken == '.' ||
					*aToken == '?' ||
					*aToken == '!'))
				{
					if (*(aToken + 1) != 0)
					{
						return 0; // not valid if not at the end of a string.
					}
					else
					{
						if (mWordno == 0)
						{
							return 0; // and not valid if the only word in the sentence
						}
						if (allupper && len > 1)
						{
							return 0; // all caps, skip
						}
						if (len < 2 && !is_alpha(aToken - 1))
						{
							return 0;
						}

						aEnd = 1; // otherwise valid, and ends the sentence
                        return 1;
                    }                
                }
                if (*aToken >= 'a' && *aToken <= 'z')
                    allupper = 0;
	        }
	        else
	        {
	            return 0;
	        }	                
	        aToken++;
	        len++;
	    }
	    if (allupper && len > 1)
	        return 0;
		if (len < 2 && !is_alpha(aToken - 1))
			return 0;
	    return 1;
	}

	void discardSentence()
	{
		int i;
		for (i = 0; i < mWordno; i++)
			free(mSentence[i]);
		mWordno = 0;
	}

	void flushSentence()
	{
		int i;
		// Make sure sentences start with an upper case character
		if (toupper(mSentence[0][0]) == mSentence[0][0])
		{
			for (i = 0; i < mWordno; i++)
				addWordcountToken(mSentence[i], i == 0, i == (mWordno - 1));
			mSentences++;
		}
		discardSentence();
	}

	void checkWord(char *aString)
	{
		int end = 0;
		if (valid(aString, end))
		{
			mSentence[mWordno] = _strdup(aString);
			mWordno++;
			if (end)
				flushSentence();
		}
		else
		{
			discardSentence();
		}
	}
	
	void wordCount(char *aString)
	{
		char temp[256];
		int p = 0;
		while (*aString)
		{
			temp[p] = *aString;
			if (*aString == '_' || 
			    *aString == '"' || 
			    *aString == ' ' || 
			    *aString == 0 || 
			    *aString < ' ' ||
				*aString > 'z')
			{ 
				temp[p] = 0;
				if (p > 0)
				{
					checkWord(temp);
				}
				p = 0;
			}
			else
			{
				p++;
			}
			aString++;
		}
		temp[p] = 0;
		checkWord(temp);
	}
};

WordCounter gWordCounter;
WELL512 gRand;




void read_raw_line(char *aBuffer, FILE * aFile)
{
    int i = 0, count = 0;
    int c;
    do
    {
        c = fgetc(aFile);
        if (c == '\r')
            c = fgetc(aFile);
        if (c == '\t')
            c = ' ';
        if (feof(aFile))
            c = '\n';
        aBuffer[i] = c;
        if (!feof(aFile) && c != '\n')
            i++;
		count++;
    }
    while (!feof(aFile) && c > 31 && !(count > 1000 && c == 32));
    
    // trim trailing spaces:
    while (i >= 0 && whitespace(aBuffer[i])) i--;
    i++;
    
    // terminate
    aBuffer[i] = 0;
}

// Skips empty and commented lines
void read_line(char *aBuffer, FILE * aFile)
{
    do
    {
        read_raw_line(aBuffer, aFile);
    }
    while (!feof(aFile) && aBuffer[0] == '#' && aBuffer[0] > 31);
}

void token(int aTokenIndex, char *aSource, char *aDestination)
{    
    while (aTokenIndex && *aSource)
    {
        while (*aSource && !whitespace(*aSource)) aSource++;
        while (*aSource && whitespace(*aSource)) aSource++;
        aTokenIndex--;
    }
    while (*aSource && !whitespace(*aSource))
    {
        *aDestination = *aSource;
        aDestination++;
        aSource++;
    }
    *aDestination = 0;
}

int tokencmp_for_qsort(const void * a, const void * b)
{
	int idx1 = *(int*)a;
	int idx2 = *(int*)b;
	return gWordCounter.mHits[idx2] - gWordCounter.mHits[idx1];
}

int findflag(int val, int flag)
{
    int i;
    int last = 0;
    for (i = 0; i < gWordCounter.mTokens; i++)
    {
        if ((gWordCounter.mFlags[i] & flag) != 0)
        {
            last = i;
            val -= gWordCounter.mHits[i];
            if (val <= 0)
                return i;
        }
    }
    return last;
}

int findstarter(int val)
{
	return findflag(val, 1);
}

int findender(int val)
{
	return findflag(val, 2);
}

int countflag(int flag)
{
	int i, ssum;
	ssum = 0;
	for (i = 0; i < gWordCounter.mTokens; i++)
	{
		if ((gWordCounter.mFlags[i] & flag) != 0) { ssum += gWordCounter.mHits[i]; }
	}
	return ssum;
}

int countinstflag(int flag)
{
	int i, ssum;
	ssum = 0;
	for (i = 0; i < gWordCounter.mTokens; i++)
	{
		if ((gWordCounter.mFlags[i] & flag) != 0) { ssum++; }
	}
	return ssum;
}

prevkeys gPrevKeys;
int gWordNumber;
int gOutputLine;
int gWantLongSentences = 10;
int gDebugLevel = 0;
#define SENTENCE_END_WEIGHT if (gWordNumber < 5) {wt /= gWantLongSentences; if (!wt) wt = 0;} else {wt += (gWordNumber * gWordNumber);}
// 
#define DEEP_LINK_WEIGHT it->second * t.depth() * t.depth() * t.depth() * t.depth();
// 1, 16, 81, 256, 625, 1296

int nextword(int w)
{
	int sum = 0, n = 0;
	gPrevKeys.shift();
	gPrevKeys.a = w;
	prevkeys t = gPrevKeys;

	if (gDebugLevel == 1)
		printf("[");


	while (t.depth())
	{
		int n = 0;
		for (auto it = gWordCounter.mLongHit[t].begin(); it != gWordCounter.mLongHit[t].end(); ++it)
		{
			if (gWordCounter.mUsed[it->first] != gOutputLine)
			{
				n++;
				int wt = DEEP_LINK_WEIGHT;
				if (gWordCounter.mFlags[it->first] & 2) SENTENCE_END_WEIGHT;
				sum += wt;
				if (gDebugLevel == 2)
					printf("%d->%s (%d)\n", t.depth(), gWordCounter.mWord[it->first], wt);
			}
		}
		if (gDebugLevel == 1)
			printf("%c=%d,",
				"?abcdef"[t.depth()], n);

		t.sink();
	}

	if (gDebugLevel == 2)
		printf("SUM: %d\n", sum);
		if (sum == 0)
	{
		if (gDebugLevel == 1)
			printf("?]");

		return findender(gRand.rand() % countflag(2));
	}
	sum = gRand.rand() % sum;
	if (gDebugLevel == 2)
		printf("RANDOM: %d\n", sum);

	int latest;
	t = gPrevKeys;
	while (t.depth())
	{
		for (auto it = gWordCounter.mLongHit[t].begin(); it != gWordCounter.mLongHit[t].end(); ++it)
		{
			if (gWordCounter.mUsed[it->first] != gOutputLine)
			{
				int wt = DEEP_LINK_WEIGHT;
				if (gWordCounter.mFlags[it->first] & 2) SENTENCE_END_WEIGHT;

				sum -= wt;

				if (gDebugLevel == 2)
					printf("(%s - %d -> %d)\n", gWordCounter.mWord[it->first], wt, sum);

				if (sum <= 0)
				{
					if (gDebugLevel == 1)
						printf("%c]", "?abcdef"[t.depth()]);
					if (gDebugLevel == 2)
						printf("SELECTED: %s (depth:%d)\n", gWordCounter.mWord[it->first], t.depth());
					return it->first;
				}
				latest = it->first;
			}
		}
		t.sink();
	}
	if (gDebugLevel == 1)
		printf("?]");

	return latest;
}

int gUsageMode = 0;

int main(int parc, char**pars)
{
	gRand.srand(0x7aa71337);

    if (parc < 2)
    {
        printf("%s textfile.txt [options]\n"
				"-i interactive mode\n"
				"-d debug output\n"
				"-a debug analysis output (really verbose)\n"
				"-l longer sentences (can be used multiple times)\n"
				"-6 max 6 word chain\n"
				"-5 max 5 word chain\n"
				"-4 max 4 word chain\n"
				"-3 max 3 word chain\n"
				"-2 max 2 word chain\n"
				"-b markdown output (book mode)\n"
				, pars[0]);
        return 0;
    }
    
	int fnameidx = -1;
	int i;
	for (i = 1; i < parc; i++)
	{
		if (pars[i][0] == '-')
		{
			switch (pars[i][1])
			{
			case 'i': gUsageMode = 1; break;
			case 'd': gDebugLevel = 1; break;
			case 'a': gDebugLevel = 2; break;
			case 'l': gWantLongSentences++; break;
			case '6': gMaxDepth = 6; break;
			case '5': gMaxDepth = 5; break;
			case '4': gMaxDepth = 4; break;
			case '3': gMaxDepth = 3; break;
			case '2': gMaxDepth = 2; break;
			case 'b': gUsageMode = 2; break;
			default:
				printf("Unknown option %s\n", pars[i]);
				return -1;
			}
		}
		else
		{
			fnameidx = i;
		}
	}

	if (fnameidx == -1)
	{
		printf("No filename given\n");
	}

	FILE * f = fopen(pars[fnameidx], "r");
    char scratch[8192];
    
	if (!f)
	{
		printf("File %s not found\n", pars[fnameidx]);
		return 0;
	}

	int lines = 0;

	printf("Processing %s\n", pars[fnameidx]);

    while (!feof(f))
    {
		lines++;
		if ((lines & 511) == 0 && gUsageMode == 1)
			printf("%d lines, %d tokens, %d sentences\r", lines, gWordCounter.mTokens, gWordCounter.mSentences);
        read_line(scratch, f);
        gWordCounter.wordCount(scratch);
    }

        
    fclose(f);
    
    int *idx = new int[gWordCounter.mTokens];
	for (i = 0; i < gWordCounter.mTokens; i++)
        idx[i] = i;
        
	qsort(idx, gWordCounter.mTokens, sizeof(int), tokencmp_for_qsort);
	printf("\n\n");
	printf("%d tokens total, %d starters, %d enders, %d sentences\n", gWordCounter.mTokens, countinstflag(1), countinstflag(2), gWordCounter.mSentences);
	printf("Most frequent words in source material:\n");
	int total = gWordCounter.mTokens;
	if (total > 99) total = 99;
	total /= 3;
    for (i = 0; i < total; i++)
    {
        printf("%2d. %-10s(%5d) %2d. %-10s(%5d) %2d. %-10s(%5d)\n",
			i+1, gWordCounter.mWord[idx[i]], gWordCounter.mHits[idx[i]],
			i+1+total, gWordCounter.mWord[idx[i+total]], gWordCounter.mHits[idx[i+total]],
			i+1+total*2, gWordCounter.mWord[idx[i+total*2]], gWordCounter.mHits[idx[i+total*2]]
			);
    }
	printf("\n");

    int s = 0, ssum = 0;

	ssum = countflag(1);
	
	gOutputLine = 0;
    for (i = 0; i < 250; i++)
    {
		gOutputLine++;
        s = findstarter(gRand.rand() % ssum);
		gPrevKeys.reset();
        
        int ps = -1;
		gWordNumber = 0;
        while ((gWordCounter.mFlags[s] & 2) == 0 && ps != s && gWordNumber < 500)
        {
            printf("%s ", gWordCounter.mWord[s]);
            ps = s;
            s = nextword(s);            
			gWordCounter.mUsed[s] = gOutputLine;

			gWordNumber++;
        }
        printf("%s ", gWordCounter.mWord[s]);
        printf("\n\n");
#ifdef _MSC_VER
		if (gUsageMode == 1)
		{
			i = 0;
			printf("More?\r");
			int c = _getch();
			if (c == 'N' || c == 'n')
			{
				printf("                \r");
				i = 300;
			}
		}
#endif        
    }
    
    return 0;
}
